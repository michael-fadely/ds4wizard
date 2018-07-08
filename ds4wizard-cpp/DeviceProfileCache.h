#pragma once

#include <string>
#include <deque>
#include <unordered_map>
#include <mutex>
#include <optional>

#include "DeviceSettings.h"
#include "DeviceProfile.h"
#include "program.h"
#include "Ds4DeviceManager.h"

class DeviceProfileCache
{
	std::shared_ptr<Ds4DeviceManager> deviceManager;
	std::unordered_map<std::string, DeviceSettings> deviceSettings;

public:
	std::recursive_mutex deviceManager_lock;
	std::recursive_mutex profiles_lock;
	std::recursive_mutex deviceSettings_lock;
	std::recursive_mutex devices_lock;
	std::deque<DeviceProfile> profiles;

	// TODO: event EventHandler Loaded;
	// TODO: event EventHandler ProfileChanged;

	DeviceProfileCache() = default;

	void setDevices(const std::shared_ptr<Ds4DeviceManager>& deviceManager);

	void load();

	/**
	 * \brief 
	 * Get a profile copy by name.
	 * \param profileName The name of the profile to get.
	 * \return A copy of the profile if found, else \c nullptr
	 */
	std::optional<DeviceProfile> getProfile(const std::string& profileName);

	/**
	 * \brief 
	 * Returns a copy of the cached settings for the specified MAC address.
	 * \param id The MAC address of the device whose settings are to be copied.
	 * \return The settings associated with the MAC address, or \c nullptr if none.
	 */
	std::optional<DeviceSettings> getSettings(const std::string& id);

	/**
	 * \brief 
	 * Adds (or replaces) settings for the specified MAC address, then saves changes to disk.
	 * \param id The MAC address of the device whose settings are being stored.
	 * \param settings The settings to be stored.
	 */
	void saveSettings(const std::string& id, const DeviceSettings& settings);

	/**
	 * \brief 
	 * Removes a profile from the profile cache.
	 * \param profile The profile to be removed.
	 */
	void removeProfile(const DeviceProfile& profile);

	/**
	 * \brief 
	 * Updates a profile and notifies all devices of the change.
	 * \param last The profile to be replaced.
	 * \param current The new profile.
	 */
	void updateProfile(const DeviceProfile& last, const DeviceProfile& current);

private:
	std::optional<DeviceProfile> findProfile(const std::string& profileName);
	void loadImpl();
	void onProfileChanged(const std::string& oldName, const std::string& newName);
};
