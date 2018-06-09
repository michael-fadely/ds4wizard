#pragma once
#include "Settings.h"

class DeviceProfileCache;

class Program
{
	static Settings lastSettings;
	static QString settingsPath_;
	static QString settingsFilePath_;
	static QString profilesPath_;
	static QString devicesFilePath_;
	inline static bool isElevated_ = false;

public:
	static DeviceProfileCache profileCache;
	static Settings settings;

	static const QString& settingsPath();
	static const QString& settingsFilePath();
	static const QString& profilesPath();
	static const QString& devicesFilePath();
	static bool isElevated();

	static void initialize();
	static void loadSettings();
	static void saveSettings();
};
