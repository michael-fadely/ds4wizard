#pragma once

#include <Windows.h>

#include <array>
#include <string>
#include <vector>

#include <gsl/span>

namespace hid
{

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

	class HidInstance
	{
		// TODO: de-windowsify HANDLE
		HANDLE handle = nullptr;
		bool is_exclusive_ = false;
		HidCaps caps_ {};
		HidAttributes attributes_ {};

	public:
		std::wstring path;
		std::wstring instance_id;
		std::wstring serial_string;
		std::vector<uint8_t> serial;

		HidInstance(const HidInstance&) = delete;
		HidInstance& operator=(const HidInstance&) = delete;

		HidInstance(const std::wstring& path, const std::wstring& instance_id, bool read_info);
		HidInstance(const std::wstring& path, bool read_info);
		HidInstance(HidInstance&& other) noexcept;

		~HidInstance();

		HidInstance& operator=(HidInstance&& other) noexcept;

		bool is_open() const;
		bool is_exclusive() const;
		const HidCaps& caps() const;
		const HidAttributes& attributes() const;

		void read_metadata();
		void get_caps();
		bool get_serial();
		bool get_attributes();
		bool get_feature(gsl::span<uint8_t> buffer) const;

		bool open(bool exclusive);
		void close();

		bool read(void* buffer, size_t length) const;
		bool read(std::vector<uint8_t>& buffer) const;

		template <size_t length>
		bool read(std::array<uint8_t, length>& buffer) const
		{
			return read(buffer.data(), buffer.size());
		}

	private:
		void get_caps(HANDLE h);
		bool get_serial(HANDLE h);
		bool get_attributes(HANDLE h);
	};

}
