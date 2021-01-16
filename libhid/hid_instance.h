#pragma once

#include <Windows.h>

#include <cstdint>
#include <string>
#include <vector>

#include <gsl/span>
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
		HidOpenFlags_t flags = 0;

		Handle handle = Handle(nullptr, true);

		HidCaps caps_ {};
		HidAttributes attributes_ {};

		OVERLAPPED overlappedIn = {};
		OVERLAPPED overlappedOut = {};

		bool pendingRead_ = false;
		bool pendingWrite_ = false;

		size_t nativeError_ = 0;

	public:
		std::wstring path;
		std::wstring instanceId;
		std::wstring serialString;
		std::vector<uint8_t> serial;

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

		bool isOpen() const;
		bool isExclusive() const;
		bool isAsync() const;
		const HidCaps& caps() const;
		const HidAttributes& attributes() const;

		bool readMetadata();
		bool readCaps();
		bool readSerial();
		bool readAttributes();
		bool getFeature(const gsl::span<uint8_t>& buffer) const;
		bool setFeature(const gsl::span<uint8_t>& buffer) const;

		bool open(HidOpenFlags_t openFlags);
		void close();

		inline auto nativeError() const
		{
			return nativeError_;
		}

		bool read(void* buffer, size_t size) const;
		bool read(const gsl::span<uint8_t>& buffer) const;
		bool read();

		bool readAsync();

		bool write(const void* buffer, size_t size) const;
		bool write(const gsl::span<const uint8_t>& buffer) const;
		bool write() const;

		bool writeAsync();

		bool asyncReadPending() const;
		bool asyncReadInProgress();
		bool asyncWritePending() const;
		bool asyncWriteInProgress();

		void cancelAsyncReadAndWait();
		void cancelAsyncWriteAndWait();

		bool setOutputReport(const gsl::span<uint8_t>& buffer) const;
		bool setOutputReport();

	private:
		void cancelAsyncAndWait(OVERLAPPED* overlapped);
		bool asyncInProgress(OVERLAPPED* overlapped);

		bool readCaps(HANDLE h);
		bool readSerial(HANDLE h);
		bool readAttributes(HANDLE h);
	};
}
