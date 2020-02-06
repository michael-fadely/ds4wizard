#include <Windows.h>
#include <hidsdi.h>
#include <hidpi.h>

#include <thread>
#include <utility>

#include "hid_handle.h"
#include "hid_instance.h"

using namespace hid;

HidInstance::HidInstance(std::wstring path, std::wstring instanceId)
	: path(std::move(path)),
	  instanceId(std::move(instanceId))
{
}

HidInstance::HidInstance(std::wstring path)
	: path(std::move(path))
{
}

HidInstance::HidInstance(HidInstance&& other) noexcept
	: flags(other.flags),
	  handle(std::move(other.handle)),
	  caps_(other.caps_),
	  attributes_(other.attributes_),
	  overlap_in(other.overlap_in),
	  overlap_out(other.overlap_out),
	  pending_read_(other.pending_read_),
	  pending_write_(other.pending_write_),
	  path(std::move(other.path)),
	  instanceId(std::move(other.instanceId)),
	  serialString(std::move(other.serialString)),
	  serial(std::move(other.serial)),
	  input_buffer(std::move(other.input_buffer)),
	  output_buffer(std::move(other.output_buffer))
{
	other.flags = 0;
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

bool HidInstance::readMetadata()
{
	if (isOpen())
	{
		return (readCaps() | readAttributes() | readSerial());
	}

	nativeError_ = 0;

#ifdef _MSC_VER
	Handle h = Handle(CreateFile(path.c_str(), GENERIC_READ,
	                             FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr), true);

	if (!h.isValid())
	{
		nativeError_ = GetLastError();
		return false;
	}
#else
	#error __FUNCTION__ not implemented on this platform.
#endif

	return (readCaps(h.nativeHandle) | readAttributes(h.nativeHandle) | readSerial(h.nativeHandle));
}

bool HidInstance::readCaps()
{
	return readCaps(handle.nativeHandle);
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

bool hid::HidInstance::setFeature(const gsl::span<uint8_t>& buffer) const
{
	if (!isOpen())
	{
		return false;
	}

	return HidD_SetFeature(handle.nativeHandle, buffer.data(), static_cast<ULONG>(buffer.size_bytes()));
}

bool HidInstance::open(HidOpenFlags_t flags)
{
	bool exclusive = !!(flags & HidOpenFlags::exclusive);

	if (exclusive != isExclusive())
	{
		close();
	}

	nativeError_ = 0;

	const uint32_t share_flags = exclusive ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE;
	const uint32_t async_flags = !!(flags & HidOpenFlags::async) ? FILE_FLAG_OVERLAPPED : 0;

	handle = Handle(CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, share_flags, nullptr,
	                           OPEN_EXISTING, async_flags, nullptr), true);

	if (!handle.isValid())
	{
		nativeError_ = GetLastError();
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

	const auto wait = WaitForSingleObject(overlap_in.hEvent, 0);

	switch (wait)
	{
		case WAIT_OBJECT_0:
			pending_read_ = false;
			return false;

		case WAIT_TIMEOUT:
			pending_read_ = true;
			return true;

		default:
			break;
	}

	DWORD bytes_read;
	pending_read_ = !GetOverlappedResult(handle.nativeHandle, &overlap_in, &bytes_read, FALSE);

	if (pending_read_)
	{
		return checkAsyncReadError();
	}

	return pending_read_;
}

bool HidInstance::checkAsyncReadError()
{
	const DWORD error = GetLastError();
	nativeError_ = 0;

	switch (error)
	{
		case ERROR_SUCCESS:
		case ERROR_IO_INCOMPLETE:
			break;

		case ERROR_IO_PENDING:
			pending_read_ = true;
			break;

		default:
			nativeError_ = error;
			close();
			return false;
	}

	return !pending_read_;
}

bool HidInstance::readAsync(void* buffer, size_t size)
{
	if (readPending())
	{
		return !checkPendingRead();
	}

	if (ReadFile(handle.nativeHandle, buffer, static_cast<DWORD>(size), nullptr, &overlap_in))
	{
		return WaitForSingleObject(overlap_in.hEvent, 0) == WAIT_OBJECT_0;
	}

	return checkAsyncReadError();
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

	const auto wait = WaitForSingleObject(overlap_out.hEvent, 0);

	switch (wait)
	{
		case WAIT_OBJECT_0:
			pending_write_ = false;
			return false;

		case WAIT_TIMEOUT:
			pending_write_ = true;
			return true;

		default:
			break;
	}

	DWORD bytes_written;
	pending_write_ = !GetOverlappedResult(handle.nativeHandle, &overlap_out, &bytes_written, FALSE);

	if (pending_write_)
	{
		return checkAsyncWriteError();
	}

	return pending_write_;
}

bool HidInstance::checkAsyncWriteError()
{
	const DWORD error = GetLastError();

	switch (error)
	{
		case ERROR_SUCCESS:
		case ERROR_IO_INCOMPLETE:
			break;

		case ERROR_IO_PENDING:
			pending_write_ = true;
			break;

		default:
			close();
			return false;
	}

	return !pending_write_;
}

bool HidInstance::writeAsync(const void* buffer, size_t size)
{
	if (writePending() && checkPendingWrite())
	{
		return false;
	}

	if (WriteFile(handle.nativeHandle, buffer, static_cast<DWORD>(size), nullptr, &overlap_out))
	{
		return WaitForSingleObject(overlap_out.hEvent, 0) == WAIT_OBJECT_0;
	}

	return checkAsyncWriteError();
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
		CancelIoEx(handle.nativeHandle, nullptr);
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

bool HidInstance::readCaps(HANDLE h)
{
	bool result = false;

	nativeError_ = 0;

#ifdef _MSC_VER
	HIDP_CAPS c = {};
	PHIDP_PREPARSED_DATA ptr = nullptr;

	if ((result = !!HidD_GetPreparsedData(h, &ptr)))
	{
		if ((result = !!HidP_GetCaps(ptr, &c)))
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
		else
		{
			nativeError_ = GetLastError();
		}

		HidD_FreePreparsedData(ptr);
	}
	else
	{
		nativeError_ = GetLastError();
	}
#else
	#error __FUNCTION__ not implemented on this platform.
#endif

	if (result)
	{
		input_buffer.resize(caps().inputReportSize);
		output_buffer.resize(caps().outputReportSize);
	}

	return result;
}

bool HidInstance::readSerial(HANDLE h)
{
	nativeError_ = 0;

	serial.clear();

#ifdef _MSC_VER
	std::array<uint8_t, 26> buffer {};

	bool result = !!HidD_GetSerialNumberString(h, buffer.data(), static_cast<ULONG>(buffer.size()));

	if (result)
	{
		serialString = std::wstring(reinterpret_cast<wchar_t*>(buffer.data()));

		// HACK: this is bad and wrong; there is a device on my system returning "2.1.8"
		if (serialString.length() < 12)
		{
			return false;
		}

		for (size_t i = 0; i < 6; i++)
		{
			const auto sub = serialString.substr(i * 2, 2);
			serial.push_back(static_cast<uint8_t>(std::stoul(sub, nullptr, 16)));
		}
	}
	else
	{
		nativeError_ = GetLastError();
	}

	return result;
#else
	#error __FUNCTION__ not implemented on this platform.
#endif
}

bool HidInstance::readAttributes(HANDLE h)
{
#ifdef _MSC_VER
	nativeError_ = 0;

	HIDD_ATTRIBUTES attributes {};
	bool result = HidD_GetAttributes(h, &attributes);

	attributes_.vendorId      = attributes.VendorID;
	attributes_.productId     = attributes.ProductID;
	attributes_.versionNumber = attributes.VersionNumber;

	if (!result)
	{
		nativeError_ = GetLastError();
	}

	return result;
#else
	#error __FUNCTION__ not implemented on this platform.
#endif
}
