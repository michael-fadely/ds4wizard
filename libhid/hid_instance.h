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

		OVERLAPPED overlap_in = {};
		OVERLAPPED overlap_out = {};

		bool pending_read_ = false;
		bool pending_write_ = false;

	public:
		std::wstring path;
		std::wstring instanceId;
		std::wstring serialString;
		std::vector<uint8_t> serial;

		std::vector<uint8_t> input_buffer;
		std::vector<uint8_t> output_buffer;

		HidInstance(const HidInstance&) = delete;
		HidInstance& operator=(const HidInstance&) = delete;

		HidInstance(std::wstring path, std::wstring instanceId, bool readInfo);
		HidInstance(std::wstring path, bool readInfo);
		HidInstance(HidInstance&& other) noexcept;

		~HidInstance();

		HidInstance& operator=(HidInstance&& other) noexcept;

		bool isOpen() const;
		bool isExclusive() const;
		bool isAsync() const;
		bool readPending() const;
		bool writePending() const;
		const HidCaps& caps() const;
		const HidAttributes& attributes() const;

		void readMetadata();
		void readCaps();
		bool readSerial();
		bool readAttributes();
		bool getFeature(const gsl::span<uint8_t>& buffer) const;

		bool open(HidOpenFlags_t flags);
		void close();

		bool read(void* buffer, size_t size) const;
		bool read(const gsl::span<uint8_t>& buffer) const;
		bool read();
		bool checkPendingRead();

		bool readAsync(void* buffer, size_t size);
		bool readAsync(const gsl::span<uint8_t>& buffer);
		bool readAsync();

		bool write(const void* buffer, size_t size) const;
		bool write(const gsl::span<const uint8_t>& buffer) const;
		bool write() const;
		bool checkPendingWrite();

		bool writeAsync(const void* buffer, size_t size);
		bool writeAsync(const gsl::span<const uint8_t>& buffer);
		bool writeAsync();

		void cancelAsync() const;

		bool setOutputReport(const gsl::span<uint8_t>& buffer) const;
		bool setOutputReport();

	private:
		void readCaps(HANDLE h);
		bool readSerial(HANDLE h);
		bool readAttributes(HANDLE h);
	};
}
