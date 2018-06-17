#include <iostream>
#include <string>
#include <devicetoggle.h>

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		return -1;
	}

	if (strcmp(argv[1], "--toggle-device") != 0)
	{
		return -2;
	}

	const std::string instanceIdA = argv[2];
	const std::wstring instanceIdW(instanceIdA.begin(), instanceIdA.end());

	try
	{
		toggleDevice(instanceIdW);
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return -3;
	}

	return 0;
}
