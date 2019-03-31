#pragma once
#include "ConnectionType.h"
#include "JsonData.h"

/**
 * \brief Structure describing program configuration.
 */
struct Settings : JsonData
{
	Settings() = default;
	Settings(const Settings&) = default;

	/**
	 * \brief Specifies the preferred connection mode for all wired/wireless devices. Default is \c ConnectionType::usb
	 */
	ConnectionType preferredConnection = ConnectionType::usb;

	/**
	 * \brief If \c true, starts the program minimized.
	 */
	bool startMinimized = false;

	/**
	 * \brief If \c true, minimizes the program to the system tray.
	 */
	bool minimizeToTray = true;

	Settings& operator=(const Settings& rhs) = default;
	bool operator==(const Settings& rhs) const;
	bool operator!=(const Settings& rhs) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
