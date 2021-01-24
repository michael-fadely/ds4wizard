#include <Windows.h>
#include <hidsdi.h>
#include <hidpi.h>

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
	  overlappedIn(other.overlappedIn),
	  overlappedOut(other.overlappedOut),
	  pendingRead_(other.pendingRead_),
	  pendingWrite_(other.pendingWrite_),
	  path(std::move(other.path)),
	  instanceId(std::move(other.instanceId)),
	  serialString(std::move(other.serialString)),
	  inputBuffer(std::move(other.inputBuffer)),
	  outputBuffer(std::move(other.outputBuffer))
{
	other.flags = 0;
}

HidInstance::~HidInstance()
{
	close();
}

HidInstance& HidInstance::operator=(HidInstance&& other) noexcept
{
	handle = std::move(other.handle);
	flags = other.flags;
	caps_ = other.caps_;
	attributes_ = other.attributes_;
	serialString = std::move(other.serialString);
	path = std::move(other.path);
	instanceId = std::move(other.instanceId);

	inputBuffer = std::move(other.inputBuffer);
	outputBuffer = std::move(other.outputBuffer);
	overlappedIn = other.overlappedIn;
	overlappedOut = other.overlappedOut;
	pendingRead_ = other.pendingRead_;
	pendingWrite_ = other.pendingWrite_;

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

	const Handle h = Handle(CreateFile(path.c_str(),
	                                   GENERIC_READ,
	                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
	                                   nullptr,
	                                   OPEN_EXISTING,
	                                   0,
	                                   nullptr),
	                        true);

	if (!h.isValid())
	{
		nativeError_ = GetLastError();
		return false;
	}

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

		const Handle h = Handle(CreateFile(path.c_str(),
		                                   GENERIC_READ | GENERIC_WRITE,
		                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
		                                   nullptr,
		                                   OPEN_EXISTING,
		                                   0,
		                                   nullptr),
		                        true);

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

bool HidInstance::setFeature(const gsl::span<uint8_t>& buffer) const
{
	if (!isOpen())
	{
		return false;
	}

	return HidD_SetFeature(handle.nativeHandle, buffer.data(), static_cast<ULONG>(buffer.size_bytes()));
}

bool HidInstance::open(HidOpenFlags_t openFlags)
{
	const bool exclusive = !!(openFlags & HidOpenFlags::exclusive);

	if (exclusive != isExclusive())
	{
		close();
	}

	nativeError_ = 0;

	const uint32_t shareFlags = exclusive ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE;
	const uint32_t asyncFlags = !!(openFlags & HidOpenFlags::async) ? FILE_FLAG_OVERLAPPED : 0;

	handle = Handle(CreateFile(path.c_str(),
	                           GENERIC_READ | GENERIC_WRITE,
	                           shareFlags,
	                           nullptr,
	                           OPEN_EXISTING,
	                           asyncFlags,
	                           nullptr),
	                true);

	if (!handle.isValid())
	{
		nativeError_ = GetLastError();
		return false;
	}

	flags = openFlags;

	if (isAsync())
	{
		//overlappedIn.hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		//overlappedOut.hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	return true;
}

void HidInstance::close()
{
	if (isAsync())
	{
		cancelAsyncReadAndWait();
		cancelAsyncWriteAndWait();

		//CloseHandle(overlappedIn.hEvent);
		//CloseHandle(overlappedOut.hEvent);

		overlappedIn = {};
		overlappedOut = {};

		pendingRead_ = false;
		pendingWrite_ = false;
	}

	if (isOpen())
	{
		handle.close();
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
	return read(inputBuffer);
}

bool HidInstance::readAsync()
{
	if (pendingRead_)
	{
		return !asyncReadInProgress();
	}

	pendingRead_ = !ReadFile(handle.nativeHandle, inputBuffer.data(), static_cast<DWORD>(inputBuffer.size()), nullptr, &overlappedIn);
	return !pendingRead_;
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
	return write(outputBuffer);
}

bool HidInstance::writeAsync()
{
	if (pendingWrite_)
	{
		return asyncWriteInProgress();
	}

	pendingWrite_ = !WriteFile(handle.nativeHandle, outputBuffer.data(), static_cast<DWORD>(outputBuffer.size()), nullptr, &overlappedOut);
	return !pendingWrite_;
}

bool HidInstance::asyncReadPending() const
{
	return pendingRead_;
}

bool HidInstance::asyncReadInProgress()
{
	if (!pendingRead_)
	{
		return false;
	}

	pendingRead_ = asyncInProgress(&overlappedIn);
	return pendingRead_;
}

bool HidInstance::asyncWritePending() const
{
	return pendingWrite_;
}

bool HidInstance::asyncWriteInProgress()
{
	if (!pendingWrite_)
	{
		return false;
	}

	pendingWrite_ = asyncInProgress(&overlappedOut);
	return pendingWrite_;
}

void HidInstance::cancelAsyncReadAndWait()
{
	if (!isOpen() || !isAsync() || !asyncReadPending())
	{
		return;
	}

	cancelAsyncAndWait(&overlappedIn);
	pendingRead_ = false;
}

void HidInstance::cancelAsyncWriteAndWait()
{
	if (!isOpen() || !isAsync() || !asyncWritePending())
	{
		return;
	}

	cancelAsyncAndWait(&overlappedOut);
	pendingWrite_ = false;
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
	return setOutputReport(outputBuffer);
}

void HidInstance::cancelAsyncAndWait(OVERLAPPED* overlapped)
{
	const bool cancelSuccess = CancelIoEx(handle.nativeHandle, overlapped) != 0;

	if (cancelSuccess)
	{
		DWORD bytesWritten = 0;
		GetOverlappedResult(handle.nativeHandle, overlapped, &bytesWritten, TRUE);
		return;
	}

	const DWORD error = GetLastError();

	switch (error)
	{
		case ERROR_NOT_FOUND:
		case ERROR_OPERATION_ABORTED:
			return;

		default:
			throw;
	}
}

bool HidInstance::asyncInProgress(OVERLAPPED* overlapped)
{
	if (!isOpen() || !isAsync())
	{
		return false;
	}

	DWORD bytesWritten = 0;
	const bool result = GetOverlappedResult(handle.nativeHandle, overlapped, &bytesWritten, FALSE) != 0;

	if (result)
	{
		return false;
	}

	const DWORD error = GetLastError();

	switch (error)
	{
		case ERROR_SUCCESS:
			return false;

		case ERROR_IO_INCOMPLETE:
		case ERROR_IO_PENDING:
			return true;

		default:
			close();
			nativeError_ = error;
			return false;
	}
}

bool HidInstance::readCaps(HANDLE h)
{
	bool result;

	nativeError_ = 0;

	HIDP_CAPS c = {};
	PHIDP_PREPARSED_DATA ptr = nullptr;

	if ((result = !!HidD_GetPreparsedData(h, &ptr)))
	{
		if ((result = !!HidP_GetCaps(ptr, &c)))
		{
			caps_.usage = c.Usage;
			caps_.usagePage = c.UsagePage;
			caps_.inputReportSize = c.InputReportByteLength;
			caps_.outputReportSize = c.OutputReportByteLength;
			caps_.featureReportSize = c.FeatureReportByteLength;
			caps_.linkCollectionNodes = c.NumberLinkCollectionNodes;
			caps_.inputButtonCaps = c.NumberInputButtonCaps;
			caps_.inputValueCaps = c.NumberInputValueCaps;
			caps_.inputDataIndices = c.NumberInputDataIndices;
			caps_.outputButtonCaps = c.NumberOutputButtonCaps;
			caps_.outputValueCaps = c.NumberOutputValueCaps;
			caps_.outputDataIndices = c.NumberOutputDataIndices;
			caps_.featureButtonCaps = c.NumberFeatureButtonCaps;
			caps_.featureValueCaps = c.NumberFeatureValueCaps;
			caps_.featureDataIndices = c.NumberFeatureDataIndices;
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

	if (result)
	{
		inputBuffer.resize(caps().inputReportSize);
		outputBuffer.resize(caps().outputReportSize);
	}

	return result;
}

bool HidInstance::readSerial(HANDLE h)
{
	nativeError_ = 0;

	// 4093 is the maximum allowed size according to Microsoft's documentation for HidD_GetSerialNumberString.
	std::vector<uint8_t> buffer(4093);

	const bool result = HidD_GetSerialNumberString(h, buffer.data(), static_cast<ULONG>(buffer.size())) != 0;

	if (!result)
	{
		nativeError_ = GetLastError();
		return result;
	}

	serialString = std::wstring(reinterpret_cast<wchar_t*>(buffer.data()), buffer.size());

	return result;
}

bool HidInstance::readAttributes(HANDLE h)
{
	nativeError_ = 0;

	HIDD_ATTRIBUTES attributes {};
	const bool result = HidD_GetAttributes(h, &attributes);

	attributes_.vendorId = attributes.VendorID;
	attributes_.productId = attributes.ProductID;
	attributes_.versionNumber = attributes.VersionNumber;

	if (!result)
	{
		nativeError_ = GetLastError();
	}

	return result;
}
