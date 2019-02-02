#pragma once
#include "Settings.h"
#include "ViGEmDriver.h"

class DeviceProfileCache;

class Program
{
	static Settings lastSettings;
	static QString settingsPath;
	static QString settingsFilePath;
	static std::string profilesPath_;
	static std::string devicesFilePath_;
	inline static bool isElevated_ = false;

public:
	static vigem::Driver driver;

	static DeviceProfileCache profileCache;
	static Settings settings;

	static const std::string& profilesPath();
	static const std::string& devicesFilePath();
	static bool isElevated();

	static void initialize();
	static void loadSettings();
	static void saveSettings();
};
