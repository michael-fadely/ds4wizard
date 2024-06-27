#pragma once

#include <Windows.h>

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "hid_handle.h"

namespace hid
{
	struct HidOpenFlags
	{
		enum T : uint32_t
		{
			none,
			exclusive = 1u << 0u,
			async     = 1u << 1u
		};
	};

	using HidOpenFlags_t = uint32_t;

	struct HidCaps
	{
		uint16_t usage;
		uint16_t usagePage;
		uint16_t inputReportSize;
		uint16_t outputReportSize;
		uint16_t featureReportSize;
		uint16_t linkCollectionNodes;
		uint16_t inputButtonCaps;
		uint16_t inputValueCaps;
		uint16_t inputDataIndices;
		uint16_t outputButtonCaps;
		uint16_t outputValueCaps;
		uint16_t outputDataIndices;
		uint16_t featureButtonCaps;
		uint16_t featureValueCaps;
		uint16_t featureDataIndices;
	};

	struct HidAttributes
	{
		uint16_t vendorId;
		uint16_t productId;
		uint16_t versionNumber;
	};

	class HidInstance
	{
		HidOpenFlags_t flags_ = HidOpenFlags::none;

		Handle handle_ = Handle(nullptr, true);

		HidCaps caps_ {};
		HidAttributes attributes_ {};

		OVERLAPPED overlappedIn_ = {};
		OVERLAPPED overlappedOut_ = {};

		bool pendingRead_ = false;
		bool pendingWrite_ = false;

		size_t nativeError_ = ERROR_SUCCESS;

	public:
		std::wstring path;
		std::wstring instanceId;
		std::wstring serialString;

		std::vector<uint8_t> inputBuffer;
		std::vector<uint8_t> outputBuffer;

		HidInstance(const HidInstance&) = delete;
		HidInstance& operator=(const HidInstance&) = delete;

		HidInstance() = default;

		HidInstance(std::wstring path, std::wstring instanceId);
		explicit HidInstance(std::wstring path);
		HidInstance(HidInstance&& other) noexcept;

		~HidInstance();

		HidInstance& operator=(HidInstance&& other) noexcept;

		[[nodiscard]] bool isOpen() const;
		[[nodiscard]] bool isExclusive() const;
		[[nodiscard]] bool isAsync() const;
		[[nodiscard]] const HidCaps& caps() const;
		[[nodiscard]] const HidAttributes& attributes() const;

		bool readMetadata();
		bool readCaps();
		bool readSerial();
		bool readAttributes();
		bool getFeature(std::span<uint8_t> buffer);
		bool setFeature(std::span<uint8_t> buffer);

		bool open(HidOpenFlags_t openFlags);
		void close();

		[[nodiscard]] inline auto nativeError() const
		{
			return nativeError_;
		}

		bool read(void* buffer, size_t size);
		bool read(std::span<uint8_t> buffer);
		bool read();

		bool readAsync();

		bool write(const void* buffer, size_t size);
		bool write(std::span<const uint8_t> buffer);
		bool write();

		bool writeAsync();

		bool asyncReadPending() const;
		bool asyncReadInProgress();
		bool asyncWritePending() const;
		bool asyncWriteInProgress();

		void cancelAsyncReadAndWait();
		void cancelAsyncWriteAndWait();

		bool setOutputReport(std::span<uint8_t> buffer);
		bool setOutputReport();

	private:
		void cancelAsyncAndWait(OVERLAPPED* overlapped);
		bool asyncInProgress(OVERLAPPED* overlapped);

		bool readCaps(HANDLE h);
		bool readSerial(HANDLE h);
		bool readAttributes(HANDLE h);
	};
}
