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
	flags         = other.flags;
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
	return !!(flags & HidOpenFlags::exclusive);
}

bool HidInstance::is_async() const
{
	return !!(flags & HidOpenFlags::async);
}

bool HidInstance::pending_read() const
{
	return pending_read_;
}

bool HidInstance::pending_write() const
{
	return pending_write_;
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

bool HidInstance::get_feature(const gsl::span<uint8_t>& buffer) const
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

bool HidInstance::open(HidOpenFlags_t flags)
{
	bool exclusive = !!(flags & HidOpenFlags::exclusive);
	if (exclusive != is_exclusive())
	{
		close();
	}

	const uint32_t share_flags = exclusive ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE;
	const uint32_t async_flags = !!(flags & HidOpenFlags::async) ? FILE_FLAG_OVERLAPPED : 0;

	handle = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, share_flags, nullptr,
	                    OPEN_EXISTING, async_flags, nullptr);

	if (handle == nullptr || handle == reinterpret_cast<HANDLE>(-1))
	{
		return false;
	}

	this->flags = flags;

	if (is_async())
	{
		overlap_in.hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		overlap_out.hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	return true;
}

void HidInstance::close()
{
	if (is_open())
	{
		CloseHandle(handle);
		handle = nullptr;
	}

	if (is_async())
	{
		CloseHandle(overlap_in.hEvent);
		CloseHandle(overlap_out.hEvent);
		pending_read_ = false;
		pending_write_ = false;
	}

	flags = 0;
}

bool HidInstance::read(void* buffer, size_t size) const
{
	if (!is_open())
	{
		return false;
	}

	return ReadFile(handle, buffer, static_cast<DWORD>(size), nullptr, nullptr) != 0;
}

bool HidInstance::read(const gsl::span<uint8_t>& buffer) const
{
	return read(buffer.data(), buffer.size());
}

bool HidInstance::read()
{
	return read(input_buffer);
}

bool HidInstance::readAsync(void* buffer, size_t size)
{
	if (pending_read_ || !ReadFile(handle, buffer, static_cast<DWORD>(size), nullptr, &overlap_in))
	{
		const DWORD error = GetLastError();

		switch (error)
		{
			case ERROR_SUCCESS:
				pending_read_ = false;
				break;

			case ERROR_IO_INCOMPLETE:
			case ERROR_IO_PENDING:
			{
				DWORD bytes_read;
				pending_read_ = !GetOverlappedResult(handle, &overlap_in, &bytes_read, FALSE);
				break;
			}

			default:
				close();
				return false;
		}
	}

	return !pending_read_;
}

bool HidInstance::readAsync(const gsl::span<uint8_t>& buffer)
{
	return readAsync(buffer.data(), buffer.size_bytes());
}

bool HidInstance::readAsync()
{
	return readAsync(input_buffer);
}

bool HidInstance::write(const void* buffer, size_t size) const
{
	return WriteFile(handle, buffer, static_cast<DWORD>(size), nullptr, nullptr);
}

bool HidInstance::write(const gsl::span<const uint8_t>& buffer) const
{
	return write(buffer.data(), buffer.size_bytes());
}

bool HidInstance::write() const
{
	return write(output_buffer);
}

bool HidInstance::writeAsync(const void* buffer, size_t size)
{
	if (!pending_write_ && WriteFile(handle, buffer, static_cast<DWORD>(size), nullptr, &overlap_out))
	{
		return false;
	}

	const DWORD error = GetLastError();

	switch (error)
	{
		case ERROR_SUCCESS:
			pending_write_ = false;
			break;

		case ERROR_IO_INCOMPLETE:
		case ERROR_IO_PENDING:
		{
			DWORD bytes_written;
			pending_write_ = !GetOverlappedResult(handle, &overlap_out, &bytes_written, FALSE);
			break;
		}

		default:
			close();
			return false;
	}

	return !pending_write_;
}

bool HidInstance::writeAsync(const gsl::span<const uint8_t>& buffer)
{
	return writeAsync(buffer.data(), buffer.size_bytes());
}

bool HidInstance::writeAsync()
{
	return writeAsync(output_buffer);
}

bool HidInstance::set_output_report(const gsl::span<uint8_t>& buffer) const
{
	if (!is_open())
	{
		return false;
	}

	return HidD_SetOutputReport(handle, reinterpret_cast<PVOID>(buffer.data()), static_cast<ULONG>(buffer.size_bytes()));
}

bool HidInstance::set_output_report()
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
