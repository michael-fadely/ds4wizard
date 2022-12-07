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
	: flags_(std::exchange(other.flags_, HidOpenFlags::none)),
	  handle_(std::move(other.handle_)),
	  caps_(other.caps_),
	  attributes_(other.attributes_),
	  overlappedIn_(other.overlappedIn_),
	  overlappedOut_(other.overlappedOut_),
	  pendingRead_(other.pendingRead_),
	  pendingWrite_(other.pendingWrite_),
	  path(std::move(other.path)),
	  instanceId(std::move(other.instanceId)),
	  serialString(std::move(other.serialString)),
	  inputBuffer(std::move(other.inputBuffer)),
	  outputBuffer(std::move(other.outputBuffer))
{
}

HidInstance::~HidInstance()
{
	close();
}

HidInstance& HidInstance::operator=(HidInstance&& other) noexcept
{
	handle_ = std::move(other.handle_);
	flags_ = std::exchange(other.flags_, HidOpenFlags::none);
	caps_ = other.caps_;
	attributes_ = other.attributes_;
	serialString = std::move(other.serialString);
	path = std::move(other.path);
	instanceId = std::move(other.instanceId);

	inputBuffer = std::move(other.inputBuffer);
	outputBuffer = std::move(other.outputBuffer);
	overlappedIn_ = other.overlappedIn_;
	overlappedOut_ = other.overlappedOut_;
	pendingRead_ = other.pendingRead_;
	pendingWrite_ = other.pendingWrite_;

	return *this;
}

bool HidInstance::isOpen() const
{
	return handle_.isValid();
}

bool HidInstance::isExclusive() const
{
	return !!(flags_ & HidOpenFlags::exclusive);
}

bool HidInstance::isAsync() const
{
	return !!(flags_ & HidOpenFlags::async);
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

	nativeError_ = ERROR_SUCCESS;

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
	return readCaps(handle_.nativeHandle);
}

bool HidInstance::readSerial()
{
	return readSerial(handle_.nativeHandle);
}

bool HidInstance::readAttributes()
{
	return readAttributes(handle_.nativeHandle);
}

bool HidInstance::getFeature(const std::span<uint8_t>& buffer)
{
	try
	{
		nativeError_ = ERROR_SUCCESS;
		bool result;

		if (isOpen())
		{
			result = !!HidD_GetFeature(handle_.nativeHandle, buffer.data(), static_cast<ULONG>(buffer.size_bytes()));
		}
		else
		{
			const Handle h = Handle(CreateFile(path.c_str(),
			                                   GENERIC_READ | GENERIC_WRITE,
			                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
			                                   nullptr,
			                                   OPEN_EXISTING,
			                                   0,
			                                   nullptr),
			                        true);

			result = h.isValid() && !!HidD_GetFeature(h.nativeHandle, buffer.data(), static_cast<ULONG>(buffer.size_bytes()));
		}

		if (!result)
		{
			nativeError_ = GetLastError();
		}

		return result;
	}
	catch (const std::exception&)
	{
		// ignored
		return false;
	}
}

bool HidInstance::setFeature(const std::span<uint8_t>& buffer)
{
	if (!isOpen())
	{
		return false;
	}

	nativeError_ = ERROR_SUCCESS;

	const bool result = !!HidD_SetFeature(handle_.nativeHandle, buffer.data(), static_cast<ULONG>(buffer.size_bytes()));

	if (!result)
	{
		nativeError_ = GetLastError();
	}

	return result;
}

bool HidInstance::open(HidOpenFlags_t openFlags)
{
	const bool exclusive = !!(openFlags & HidOpenFlags::exclusive);

	if (exclusive != isExclusive())
	{
		close();
	}

	nativeError_ = ERROR_SUCCESS;

	const uint32_t shareFlags = exclusive ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE;
	const uint32_t asyncFlags = !!(openFlags & HidOpenFlags::async) ? FILE_FLAG_OVERLAPPED : 0;

	handle_ = Handle(CreateFile(path.c_str(),
	                            GENERIC_READ | GENERIC_WRITE,
	                            shareFlags,
	                            nullptr,
	                            OPEN_EXISTING,
	                            asyncFlags,
	                            nullptr),
	                 true);

	if (!handle_.isValid())
	{
		nativeError_ = GetLastError();
		return false;
	}

	flags_ = openFlags;

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

		overlappedIn_ = {};
		overlappedOut_ = {};

		pendingRead_ = false;
		pendingWrite_ = false;
	}

	if (isOpen())
	{
		handle_.close();
	}

	flags_ = HidOpenFlags::none;
}

bool HidInstance::read(void* buffer, size_t size)
{
	if (!isOpen())
	{
		return false;
	}

	nativeError_ = ERROR_SUCCESS;
	const bool result = !!ReadFile(handle_.nativeHandle, buffer, static_cast<DWORD>(size), nullptr, nullptr);

	if (!result)
	{
		nativeError_ = GetLastError();
	}

	return result;
}

bool HidInstance::read(const std::span<uint8_t>& buffer)
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

	const bool result = !!ReadFile(handle_.nativeHandle,
	                               inputBuffer.data(),
	                               static_cast<DWORD>(inputBuffer.size()),
	                               nullptr,
	                               &overlappedIn_);

	if (!result)
	{
		const DWORD error = GetLastError();

		if (error == ERROR_IO_PENDING)
		{
			pendingRead_ = true;
			return true;
		}

		nativeError_ = error;
	}

	return result;
}

bool HidInstance::write(const void* buffer, size_t size)
{
	if (!isOpen())
	{
		return false;
	}

	nativeError_ = ERROR_SUCCESS;
	const bool result = !!WriteFile(handle_.nativeHandle, buffer, static_cast<DWORD>(size), nullptr, nullptr);

	if (!result)
	{
		nativeError_ = GetLastError();
	}

	return result;
}

bool HidInstance::write(const std::span<const uint8_t>& buffer)
{
	return write(buffer.data(), buffer.size_bytes());
}

bool HidInstance::write()
{
	return write(outputBuffer);
}

bool HidInstance::writeAsync()
{
	if (pendingWrite_)
	{
		return asyncWriteInProgress();
	}

	const bool result = !!WriteFile(handle_.nativeHandle,
	                                outputBuffer.data(),
	                                static_cast<DWORD>(outputBuffer.size()),
	                                nullptr,
	                                &overlappedOut_);

	if (!result)
	{
		const DWORD error = GetLastError();

		if (error == ERROR_IO_PENDING)
		{
			pendingWrite_ = true;
			return true;
		}

		nativeError_ = error;
	}

	return result;
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

	pendingRead_ = asyncInProgress(&overlappedIn_);
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

	pendingWrite_ = asyncInProgress(&overlappedOut_);
	return pendingWrite_;
}

void HidInstance::cancelAsyncReadAndWait()
{
	if (!isOpen() || !isAsync() || !asyncReadPending())
	{
		return;
	}

	cancelAsyncAndWait(&overlappedIn_);
	pendingRead_ = false;
}

void HidInstance::cancelAsyncWriteAndWait()
{
	if (!isOpen() || !isAsync() || !asyncWritePending())
	{
		return;
	}

	cancelAsyncAndWait(&overlappedOut_);
	pendingWrite_ = false;
}

bool HidInstance::setOutputReport(const std::span<uint8_t>& buffer)
{
	if (!isOpen())
	{
		return false;
	}

	nativeError_ = ERROR_SUCCESS;

	const bool result = !!HidD_SetOutputReport(handle_.nativeHandle, buffer.data(), static_cast<ULONG>(buffer.size_bytes()));

	if (!result)
	{
		nativeError_ = GetLastError();
	}

	return result;
}

bool HidInstance::setOutputReport()
{
	return setOutputReport(outputBuffer);
}

void HidInstance::cancelAsyncAndWait(OVERLAPPED* overlapped)
{
	const bool cancelSuccess = CancelIoEx(handle_.nativeHandle, overlapped) != 0;

	if (cancelSuccess)
	{
		DWORD bytesWritten = 0;
		GetOverlappedResult(handle_.nativeHandle, overlapped, &bytesWritten, TRUE);
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

	// TODO: Make use of HasOverlappedIoCompleted

	DWORD bytesWritten = 0;
	const bool result = GetOverlappedResult(handle_.nativeHandle, overlapped, &bytesWritten, FALSE) != 0;

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

	nativeError_ = ERROR_SUCCESS;

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
	nativeError_ = ERROR_SUCCESS;

	// 4093 is the maximum allowed size according to Microsoft's documentation for HidD_GetSerialNumberString.
	// Using std::vector because this is *quite* large.
	constexpr size_t serialNumberStringMaxLength = 4093;
	std::vector<uint8_t> buffer(serialNumberStringMaxLength);

	const bool result = HidD_GetSerialNumberString(h, buffer.data(), static_cast<ULONG>(buffer.size())) != 0;

	if (!result)
	{
		nativeError_ = GetLastError();
		return result;
	}

	const auto* wstr_ptr = reinterpret_cast<wchar_t*>(buffer.data());
	const size_t length = wcsnlen(wstr_ptr, buffer.size());
	serialString = std::wstring(wstr_ptr, length);

	return result;
}

bool HidInstance::readAttributes(HANDLE h)
{
	nativeError_ = ERROR_SUCCESS;

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
