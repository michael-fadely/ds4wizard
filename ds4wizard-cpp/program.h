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
	/**
	 * \brief ViGEm driver instance used for endpoint emulation (XInput, DualShock 4).
	 * \sa vigem::Driver
	 */
	static vigem::Driver driver;

	/**
	 * \brief Profiles currently tracked and managed by the program.
	 * \sa DeviceProfileCache
	 */
	static DeviceProfileCache profileCache;

	/**
	 * \brief Currently active program settings.
	 * \sa Settings
	 */
	static Settings settings;

	/**
	 * \brief Filesystem path to search for profiles.
	 */
	static const std::string& profilesPath();

	/**
	 * \brief Filesystem path to the per-device configuration file.
	 */
	static const std::string& devicesFilePath();

	/**
	 * \brief Indicates whether or not the program is running with elevated privileges.
	 * \return \c true if the application is running with elevated privileges.
	 */
	static bool isElevated();

	/**
	 * \brief Performs startup initialization.
	 */
	static void initialize();

	/**
	 * \brief Loads program settings.
	 * \sa Settings, settings
	 */
	static void loadSettings();

	/**
	 * \brief Saves program settings to disk if changed.
	 * \sa Settings, settings
	 */
	static void saveSettings();
};
