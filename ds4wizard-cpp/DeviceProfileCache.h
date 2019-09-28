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

/**
 * \brief A class which manages device profiles, changes to those profiles, and notifying relevant devices of those changes.
 */
class DeviceProfileCache
{
	std::shared_ptr<Ds4DeviceManager> deviceManager;
	std::unordered_map<std::string, DeviceSettings> deviceSettings;

public:
	// TODO: all of these should be private
	std::recursive_mutex deviceManager_lock;
	std::recursive_mutex profiles_lock;
	std::recursive_mutex deviceSettings_lock;
	std::recursive_mutex devices_lock;
	std::deque<DeviceProfile> profiles;

	/**
	 * \brief Event raised when a new profile is added.
	 * Parameters are: profile, index
	 */
	Event<DeviceProfileCache, const DeviceProfile&, int> profileAdded;

	// TODO: modify in-place
	/**
	 * \brief Event raised when a profile is modified.
	 * Parameters are: old, new, old index, new index
	 */
	Event<DeviceProfileCache, const DeviceProfile&, const DeviceProfile&, int, int> profileChanged;

	/**
	 * \brief Event raised when a profile is permanently removed.
	 * Parameters are: profile, index
	 */
	Event<DeviceProfileCache, const DeviceProfile&, int> profileRemoved;

	DeviceProfileCache() = default;

	/**
	 * \brief Sets the device manager instance to be used when notifying devices of profile changes.
	 * \param deviceManager The device manager instance.
	 */
	void setDevices(const std::shared_ptr<Ds4DeviceManager>& deviceManager);

	/**
	 * \brief Load profiles from disk.
	 */
	void load();

	/**
	 * \brief Get a profile copy by name.
	 * \param profileName The name of the profile to get.
	 * \return A copy of the profile if found, else \c std::nullopt
	 */
	std::optional<DeviceProfile> getProfile(const std::string& profileName);

	/**
	 * \brief Returns a copy of the cached settings for the specified MAC address.
	 * \param id The MAC address of the device whose settings are to be copied.
	 * \return The settings associated with the MAC address, or \c std::nullopt if none.
	 */
	std::optional<DeviceSettings> getSettings(const std::string& id);

	/**
	 * \brief Adds (or replaces) settings for the specified MAC address, then saves changes to disk.
	 * \param id The MAC address of the device whose settings are being stored.
	 * \param settings The settings to be stored.
	 */
	void saveSettings(const std::string& id, const DeviceSettings& settings);

	/**
	 * \brief Adds a profile.
	 * \param current The new profile.
	 * \return \c true on success, \c false if, for example, a profile with that name exists.
	 */
	bool addProfile(const DeviceProfile& current);

	/**
	 * \brief Removes a profile from the profile cache.
	 * \param profile The profile to be removed.
	 */
	void removeProfile(const DeviceProfile& profile);

	/**
	 * \brief Updates a profile and notifies all devices of the change.
	 * \param last The profile to be replaced.
	 * \param current The new profile.
	 */
	void updateProfile(const DeviceProfile& last, const DeviceProfile& current);

private:
	std::optional<DeviceProfile> findProfile(const std::string& profileName);
	void loadImpl();
	void onProfileChanged(const std::string& oldName, const std::string& newName);
};
