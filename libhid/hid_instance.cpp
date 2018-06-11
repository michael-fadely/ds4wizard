#include <Windows.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <utility>
#include "hid_instance.h"
#include "hid_handle.h"
#include <thread>

using namespace hid;

HidInstance::HidInstance(std::wstring path, std::wstring instanceId, bool readInfo)
	: path(std::move(path)),
	  instanceId(std::move(instanceId))
{
	if (readInfo)
	{
		readMetadata();
	}
}

HidInstance::HidInstance(std::wstring path, bool readInfo)
	: path(std::move(path))
{
	if (readInfo)
	{
		readMetadata();
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
	handle       = std::move(other.handle);
	flags        = other.flags;
	caps_        = other.caps_;
	attributes_  = other.attributes_;
	serialString = std::move(other.serialString);
	serial       = std::move(other.serial);
	path         = std::move(other.path);
	instanceId   = std::move(other.instanceId);

	input_buffer   = std::move(other.input_buffer);
	output_buffer  = std::move(other.output_buffer);
	overlap_in     = other.overlap_in;
	overlap_out    = other.overlap_out;
	pending_read_  = other.pending_read_;
	pending_write_ = other.pending_write_;

	other.flags = 0;

	return *this;
}

bool HidInstance::isOpen() const
{
	return handle.isValid();
}

bool HidInstance::isExclusive() const
{
	return !!(flags & HidOpenFlags::exclusive);
}

bool HidInstance::isAsync() const
{
	return !!(flags & HidOpenFlags::async);
}

bool HidInstance::readPending() const
{
	return pending_read_;
}

bool HidInstance::writePending() const
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

void HidInstance::readMetadata()
{
	if (isOpen())
	{
		readCaps();
		readAttributes();
		readSerial();
		return;
	}

#ifdef _MSC_VER
	Handle h = Handle(CreateFile(path.c_str(), GENERIC_READ,
	                             FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr), true);

	if (!h.isValid())
	{
		const std::string path_a(path.begin(), path.end());
		const std::string message = "Failed to open handle to device: " + path_a;
		throw std::runtime_error(message);
	}
#else
	#error __FUNCTION__ not implemented on this platform.
#endif

	readCaps(h.nativeHandle);
	readAttributes(h.nativeHandle);
	readSerial(h.nativeHandle);
}

void HidInstance::readCaps()
{
	readCaps(handle.nativeHandle);
}

bool HidInstance::readSerial()
{
	return readSerial(handle.nativeHandle);
}

bool HidInstance::readAttributes()
{
	return readAttributes(handle.nativeHandle);
}

bool HidInstance::getFeature(const gsl::span<uint8_t>& buffer) const
{
	try
	{
		if (isOpen())
		{
			return HidD_GetFeature(handle.nativeHandle, buffer.data(), static_cast<ULONG>(buffer.size_bytes()));
		}

		Handle h = Handle(CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE,
		                             FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr), true);

		if (!h.isValid())
		{
			return false;
		}

		return HidD_GetFeature(h.nativeHandle, buffer.data(), static_cast<ULONG>(buffer.size_bytes()));
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
	if (exclusive != isExclusive())
	{
		close();
	}

	const uint32_t share_flags = exclusive ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE;
	const uint32_t async_flags = !!(flags & HidOpenFlags::async) ? FILE_FLAG_OVERLAPPED : 0;

	handle = Handle(CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, share_flags, nullptr,
	                           OPEN_EXISTING, async_flags, nullptr), true);

	if (!handle.isValid())
	{
		return false;
	}

	this->flags = flags;

	if (isAsync())
	{
		overlap_in.hEvent  = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		overlap_out.hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	return true;
}

void HidInstance::close()
{
	if (isOpen())
	{
		handle.close();
	}

	if (isAsync())
	{
		cancelAsync();
		CloseHandle(overlap_in.hEvent);
		CloseHandle(overlap_out.hEvent);
		pending_read_  = false;
		pending_write_ = false;
	}

	flags = 0;
}

bool HidInstance::read(void* buffer, size_t size) const
{
	if (!isOpen())
	{
		return false;
	}

	return ReadFile(handle.nativeHandle, buffer, static_cast<DWORD>(size), nullptr, nullptr) != 0;
}

bool HidInstance::read(const gsl::span<uint8_t>& buffer) const
{
	return read(buffer.data(), buffer.size());
}

bool HidInstance::read()
{
	return read(input_buffer);
}

bool HidInstance::checkPendingRead()
{
	if (!pending_read_)
	{
		return false;
	}

	DWORD bytes_read;
	pending_read_ = !GetOverlappedResult(handle.nativeHandle, &overlap_in, &bytes_read, FALSE);
	return pending_read_;
}

bool HidInstance::readAsync(void* buffer, size_t size)
{
	if (readPending())
	{
		checkPendingRead();
		return false;
	}

	if (ReadFile(handle.nativeHandle, buffer, static_cast<DWORD>(size), nullptr, &overlap_in))
	{
		/*pending_read_ = true;
		checkPendingRead();*/
		return true;
	}

	const DWORD error = GetLastError();

	switch (error)
	{
		case ERROR_SUCCESS:
		case ERROR_IO_INCOMPLETE:
			break;

		case ERROR_IO_PENDING:
			pending_read_ = true;
			checkPendingRead();
			break;

		default:
			close();
			return false;
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
	return WriteFile(handle.nativeHandle, buffer, static_cast<DWORD>(size), nullptr, nullptr);
}

bool HidInstance::write(const gsl::span<const uint8_t>& buffer) const
{
	return write(buffer.data(), buffer.size_bytes());
}

bool HidInstance::write() const
{
	return write(output_buffer);
}

bool HidInstance::checkPendingWrite()
{
	if (!pending_write_)
	{
		return false;
	}

	DWORD bytes_written;
	pending_write_ = !GetOverlappedResult(handle.nativeHandle, &overlap_out, &bytes_written, FALSE);
	return pending_write_;
}

bool HidInstance::writeAsync(const void* buffer, size_t size)
{
	if (writePending())
	{
		checkPendingWrite();
		return false;
	}

	if (WriteFile(handle.nativeHandle, buffer, static_cast<DWORD>(size), nullptr, &overlap_out))
	{
		/*pending_write_ = true;
		checkPendingWrite();*/
		return true;
	}

	const DWORD error = GetLastError();

	switch (error)
	{
		case ERROR_SUCCESS:
		case ERROR_IO_INCOMPLETE:
			break;

		case ERROR_IO_PENDING:
			pending_write_ = true;
			checkPendingWrite();
			break;

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

void HidInstance::cancelAsync() const
{
	if (isOpen() && isAsync())
	{
		CancelIo(handle.nativeHandle);
	}
}

bool HidInstance::setOutputReport(const gsl::span<uint8_t>& buffer) const
{
	if (!isOpen())
	{
		return false;
	}

	return HidD_SetOutputReport(handle.nativeHandle, reinterpret_cast<PVOID>(buffer.data()), static_cast<ULONG>(buffer.size_bytes()));
}

bool HidInstance::setOutputReport()
{
	return setOutputReport(output_buffer);
}

void HidInstance::readCaps(HANDLE h)
{
#ifdef _MSC_VER
	HIDP_CAPS c              = {};
	PHIDP_PREPARSED_DATA ptr = nullptr;

	if (HidD_GetPreparsedData(h, &ptr))
	{
		if (HidP_GetCaps(ptr, &c))
		{
			caps_.usage               = c.Usage;
			caps_.usagePage           = c.UsagePage;
			caps_.inputReportSize     = c.InputReportByteLength;
			caps_.outputReportSize    = c.OutputReportByteLength;
			caps_.featureReportSize   = c.FeatureReportByteLength;
			caps_.linkCollectionNodes = c.NumberLinkCollectionNodes;
			caps_.inputButtonCaps     = c.NumberInputButtonCaps;
			caps_.inputValueCaps      = c.NumberInputValueCaps;
			caps_.inputDataIndices    = c.NumberInputDataIndices;
			caps_.outputButtonCaps    = c.NumberOutputButtonCaps;
			caps_.outputValueCaps     = c.NumberOutputValueCaps;
			caps_.outputDataIndices   = c.NumberOutputDataIndices;
			caps_.featureButtonCaps   = c.NumberFeatureButtonCaps;
			caps_.featureValueCaps    = c.NumberFeatureValueCaps;
			caps_.featureDataIndices  = c.NumberFeatureDataIndices;
		}

		HidD_FreePreparsedData(ptr);
	}
#else
	#error __FUNCTION__ not implemented on this platform.
#endif

	input_buffer.resize(caps().inputReportSize);
	output_buffer.resize(caps().outputReportSize);
}

bool HidInstance::readSerial(HANDLE h)
{
	serial.clear();

#ifdef _MSC_VER
	std::array<uint8_t, 26> buffer {};

	bool result = HidD_GetSerialNumberString(h, buffer.data(), static_cast<ULONG>(buffer.size()));

	if (result)
	{
		serialString = std::wstring(reinterpret_cast<wchar_t*>(buffer.data()));

		for (size_t i = 0; i < 6; i++)
		{
			const auto sub = serialString.substr(i * 2, 2);
			serial.push_back(static_cast<uint8_t>(std::stoul(sub, nullptr, 16)));
		}
	}

	return result;
#else
	#error __FUNCTION__ not implemented on this platform.
#endif
}

bool HidInstance::readAttributes(HANDLE h)
{
#ifdef _MSC_VER
	HIDD_ATTRIBUTES attributes {};
	bool result = HidD_GetAttributes(h, &attributes);

	attributes_.vendorId      = attributes.VendorID;
	attributes_.productId     = attributes.ProductID;
	attributes_.versionNumber = attributes.VersionNumber;

	return result;
#else
	#error __FUNCTION__ not implemented on this platform.
#endif
}
