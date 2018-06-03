#pragma once

#include <Windows.h>

#include <cstdint>
#include <string>
#include <vector>

#include <gsl/span>

namespace hid
{
	struct HidOpenFlags
	{
		enum T : uint32_t
		{
			none,
			exclusive = 1u << 0,
			async     = 1u << 1
		};
	};

	using HidOpenFlags_t = uint32_t;

	struct HidCaps
	{
		uint16_t usage;
		uint16_t usage_page;
		uint16_t input_report_size;
		uint16_t output_report_size;
		uint16_t feature_report_size;
		uint16_t link_collection_nodes;
		uint16_t input_button_caps;
		uint16_t input_value_caps;
		uint16_t input_data_indices;
		uint16_t output_button_caps;
		uint16_t output_value_caps;
		uint16_t output_data_indices;
		uint16_t feature_button_caps;
		uint16_t feature_value_caps;
		uint16_t feature_data_indices;
	};

	struct HidAttributes
	{
		uint16_t vendor_id;
		uint16_t product_id;
		uint16_t version_number;
	};

	class Handle
	{
	public:
		bool owner = false;
		HANDLE nativeHandle = nullptr;

		Handle() = default;
		Handle(const Handle&) = default;

		Handle(Handle&& rhs) noexcept;
		Handle(HANDLE h, bool owner = false);
		~Handle();

		void close();

		bool operator==(const Handle& rhs) const;
		bool operator!=(const Handle& rhs) const;

		Handle& operator=(const Handle&) = default;
		Handle& operator=(Handle&& rhs) noexcept;

		bool isValid() const;
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
		std::wstring instance_id;
		std::wstring serial_string;
		std::vector<uint8_t> serial;

		std::vector<uint8_t> input_buffer;
		std::vector<uint8_t> output_buffer;

		HidInstance(const HidInstance&) = delete;
		HidInstance& operator=(const HidInstance&) = delete;

		HidInstance(const std::wstring& path, const std::wstring& instance_id, bool read_info);
		HidInstance(const std::wstring& path, bool read_info);
		HidInstance(HidInstance&& other) noexcept;

		~HidInstance();

		HidInstance& operator=(HidInstance&& other) noexcept;

		bool is_open() const;
		bool is_exclusive() const;
		bool is_async() const;
		bool pending_read() const;
		bool pending_write() const;
		const HidCaps& caps() const;
		const HidAttributes& attributes() const;

		void read_metadata();
		void get_caps();
		bool get_serial();
		bool get_attributes();
		bool get_feature(const gsl::span<uint8_t>& buffer) const;

		bool open(HidOpenFlags_t flags);
		void close();

		bool read(void* buffer, size_t size) const;
		bool read(const gsl::span<uint8_t>& buffer) const;
		bool read();

		bool readAsync(void* buffer, size_t size);
		bool readAsync(const gsl::span<uint8_t>& buffer);
		bool readAsync();

		bool write(const void* buffer, size_t size) const;
		bool write(const gsl::span<const uint8_t>& buffer) const;
		bool write() const;

		bool writeAsync(const void* buffer, size_t size);
		bool writeAsync(const gsl::span<const uint8_t>& buffer);
		bool writeAsync();

		bool set_output_report(const gsl::span<uint8_t>& buffer) const;
		bool set_output_report();

	private:
		void get_caps(HANDLE h);
		bool get_serial(HANDLE h);
		bool get_attributes(HANDLE h);
	};

}
