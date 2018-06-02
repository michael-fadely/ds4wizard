#include <Windows.h>
#include <hidsdi.h>
#include <hidpi.h>
#include "hid_instance.h"

using namespace hid;

HidInstance::HidInstance(const std::wstring& path, const std::wstring& instance_id, bool read_info)
	: HidInstance(path, read_info)
{
	this->instance_id = instance_id;
}

HidInstance::HidInstance(const std::wstring& path, bool read_info)
{
	this->path = path;

	if (read_info)
	{
		read_metadata();
	}
}

HidInstance::HidInstance(HidInstance&& other) noexcept
{
	*this = std::move(other);
}

HidInstance::~HidInstance()
{
	close();
}

HidInstance& HidInstance::operator=(HidInstance&& other) noexcept
{
	handle        = other.handle;
	is_exclusive_ = other.is_exclusive_;
	caps_         = other.caps_;
	attributes_   = other.attributes_;
	serial_string = std::move(other.serial_string);
	serial        = std::move(other.serial);
	path          = std::move(other.path);
	instance_id   = std::move(other.instance_id);

	other.handle = nullptr;

	return *this;
}

bool HidInstance::is_open() const
{
	return handle != nullptr &&
		   handle != reinterpret_cast<HANDLE>(-1);
}

bool HidInstance::is_exclusive() const
{
	return is_exclusive_;
}

const HidCaps& HidInstance::caps() const
{
	return caps_;
}

const HidAttributes& HidInstance::attributes() const
{
	return attributes_;
}

void HidInstance::read_metadata()
{
	if (is_open())
	{
		get_caps();
		get_attributes();
		//get_serial();
		return;
	}

#ifdef _MSC_VER
	auto h = CreateFile(path.c_str(), GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	if (h == nullptr || h == reinterpret_cast<HANDLE>(-1))
	{
		const std::string path_a(path.begin(), path.end());
		const std::string message = "Failed to open handle to device: " + path_a;
		throw std::runtime_error(message);
	}
#else
	#error __FUNCTION__ not implemented on this platform.
#endif

	get_caps(h);
	get_attributes(h);
	//get_serial(h);

	CloseHandle(h);
}

void HidInstance::get_caps()
{
	get_caps(handle);

	input_buffer.resize(caps().input_report_size);
	output_buffer.resize(caps().output_report_size);
}

bool HidInstance::get_serial()
{
	return get_serial(handle);
}

bool HidInstance::get_attributes()
{
	return get_attributes(handle);
}

bool HidInstance::get_feature(gsl::span<uint8_t> buffer) const
{
	try
	{
		if (is_open())
		{
			return HidD_GetFeature(handle, buffer.data(), static_cast<ULONG>(buffer.size_bytes()));
		}

		auto ptr = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

		if (!ptr || ptr == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		return HidD_GetFeature(ptr, buffer.data(), static_cast<ULONG>(buffer.size_bytes()));
	}
	catch (const std::exception&)
	{
		// ignored
		return false;
	}
}

bool HidInstance::open(bool exclusive)
{
	if (exclusive != is_exclusive())
	{
		close();
	}

	const uint32_t share_flags = exclusive ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE;

	handle = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, share_flags, nullptr,
		OPEN_EXISTING, /*FILE_FLAG_OVERLAPPED*/ 0, nullptr);

	if (handle == nullptr || handle == reinterpret_cast<HANDLE>(-1))
	{
		return false;
	}

	is_exclusive_ = exclusive;
	return true;
}

void HidInstance::close()
{
	if (is_open())
	{
		CloseHandle(handle);
		handle = nullptr;
	}

	is_exclusive_ = false;
}

bool HidInstance::read(void* buffer, size_t length) const
{
	if (!is_open())
	{
		return false;
	}

	DWORD read = 0;
	return ReadFile(handle, buffer, static_cast<DWORD>(length), &read, nullptr) != 0;
}

bool HidInstance::read(gsl::span<uint8_t> buffer) const
{
	return read(buffer.data(), buffer.size());
}

bool HidInstance::set_output_report(gsl::span<uint8_t> buffer) const
{
	if (!is_open())
	{
		return false;
	}

	return HidD_SetOutputReport(handle, buffer.data(), buffer.size_bytes());
}

bool HidInstance::set_output_report() const
{
	return set_output_report(output_buffer);
}

void HidInstance::get_caps(HANDLE h)
{
#ifdef _MSC_VER
	HIDP_CAPS c = {};
	PHIDP_PREPARSED_DATA ptr = nullptr;

	if (HidD_GetPreparsedData(h, &ptr))
	{
		if (HidP_GetCaps(ptr, &c))
		{
			caps_.usage                 = c.Usage;
			caps_.usage_page            = c.UsagePage;
			caps_.input_report_size     = c.InputReportByteLength;
			caps_.output_report_size    = c.OutputReportByteLength;
			caps_.feature_report_size   = c.FeatureReportByteLength;
			caps_.link_collection_nodes = c.NumberLinkCollectionNodes;
			caps_.input_button_caps     = c.NumberInputButtonCaps;
			caps_.input_value_caps      = c.NumberInputValueCaps;
			caps_.input_data_indices    = c.NumberInputDataIndices;
			caps_.output_button_caps    = c.NumberOutputButtonCaps;
			caps_.output_value_caps     = c.NumberOutputValueCaps;
			caps_.output_data_indices   = c.NumberOutputDataIndices;
			caps_.feature_button_caps   = c.NumberFeatureButtonCaps;
			caps_.feature_value_caps    = c.NumberFeatureValueCaps;
			caps_.feature_data_indices  = c.NumberFeatureDataIndices;
		}

		HidD_FreePreparsedData(ptr);
	}
#else
	#error __FUNCTION__ not implemented on this platform.
#endif
}

bool HidInstance::get_serial(HANDLE h)
{
	serial.clear();

#ifdef _MSC_VER
	std::array<uint8_t, 26> buffer {};

	bool result = HidD_GetSerialNumberString(h, buffer.data(), static_cast<ULONG>(buffer.size()));

	if (result)
	{
		serial_string = std::wstring(reinterpret_cast<wchar_t*>(buffer.data()));

		for (size_t i = 0; i < 6; i++)
		{
			const auto sub = serial_string.substr(i * 2, 2);
			serial.push_back(static_cast<uint8_t>(std::stoul(sub, nullptr, 16)));
		}
	}

	return result;
#else
	#error __FUNCTION__ not implemented on this platform.
#endif
}

bool HidInstance::get_attributes(HANDLE h)
{
#ifdef _MSC_VER
	HIDD_ATTRIBUTES attributes {};
	bool result = HidD_GetAttributes(h, &attributes);

	attributes_.vendor_id = attributes.VendorID;
	attributes_.product_id = attributes.ProductID;
	attributes_.version_number = attributes.VersionNumber;

	return result;
#else
	#error __FUNCTION__ not implemented on this platform.
#endif
}
