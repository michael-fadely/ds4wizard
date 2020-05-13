#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>
#include <optional>
#include <vector>
#include <string>

#include <hid_util.h>
#include <hid_instance.h>

static std::shared_ptr<hid::HidInstance> device = nullptr;
static std::optional<uint16_t> vendorId;
static std::optional<uint16_t> productId;

bool enumFunc(std::shared_ptr<hid::HidInstance> inst)
{
	const auto& attr = inst->attributes();

	if ((vendorId.has_value() && attr.vendorId != *vendorId) ||
	    (productId.has_value() && attr.productId != *productId))
	{
		return false;
	}

	const auto& caps = inst->caps();

	std::wcout << L"Device found: " << inst->path << std::endl;
	std::cout << "Diagnostics: " << std::endl;

	std::cout << "\tusage:               " << caps.usage << std::endl;
	std::cout << "\tusagePage:           " << caps.usagePage << std::endl;
	std::cout << "\tinputReportSize:     " << caps.inputReportSize << std::endl;
	std::cout << "\toutputReportSize:    " << caps.outputReportSize << std::endl;
	std::cout << "\tfeatureReportSize:   " << caps.featureReportSize << std::endl;
	std::cout << "\tlinkCollectionNodes: " << caps.linkCollectionNodes << std::endl;
	std::cout << "\tinputButtonCaps:     " << caps.inputButtonCaps << std::endl;
	std::cout << "\tinputValueCaps:      " << caps.inputValueCaps << std::endl;
	std::cout << "\tinputDataIndices:    " << caps.inputDataIndices << std::endl;
	std::cout << "\toutputButtonCaps:    " << caps.outputButtonCaps << std::endl;
	std::cout << "\toutputValueCaps:     " << caps.outputValueCaps << std::endl;
	std::cout << "\toutputDataIndices:   " << caps.outputDataIndices << std::endl;
	std::cout << "\tfeatureButtonCaps:   " << caps.featureButtonCaps << std::endl;
	std::cout << "\tfeatureValueCaps:    " << caps.featureValueCaps << std::endl;
	std::cout << "\tfeatureDataIndices:  " << caps.featureDataIndices << std::endl;

	device = std::move(inst);
	return true;
}

int main(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
	{
		std::string arg(argv[i]);

		if (i + 1 >= argc)
		{
			break;
		}

		if (arg == "--vendor-id")
		{
			vendorId = static_cast<uint16_t>(std::stoi(argv[++i], nullptr, 16));
		}
		else if (arg == "--product-id")
		{
			productId = static_cast<uint16_t>(std::stoi(argv[++i], nullptr, 16));
		}
	}

	if (!vendorId.has_value() && !productId.has_value())
	{
		printf("fam look you gotta give me --vendor-id and/or --product-id (hex without 0x because I'm lazy)\n");
		return -1;
	}

	hid::enumerateHid(enumFunc);

	if (device == nullptr)
	{
		printf("couldn't find shit bye\n");
		return 0;
	}

	auto size = device->caps().inputReportSize;
	std::vector<uint8_t> buffer(size);

	device->open(false);

	while (device->read(buffer))
	{
		for (int i = 0; i < size; i++)
		{
			printf("%02X ", buffer[i]);
		}

		printf("\r");
	}

	device = nullptr;
	return 0;
}
