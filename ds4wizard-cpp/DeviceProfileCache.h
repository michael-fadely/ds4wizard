#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include <mutex>

#include "DeviceSettings.h"
#include "DeviceProfile.h"
#include "program.h"

class DeviceProfileCache
{
	// TODO
	//Ds4DeviceManager devices;
	std::unordered_map<std::string, DeviceSettings> deviceSettings;


public:
	std::recursive_mutex profile_lock;
	std::recursive_mutex deviceSettings_lock;
	std::recursive_mutex devices_lock;
	std::list<DeviceProfile> profiles;

	// TODO
	//event EventHandler Loaded;
	// TODO
	//event EventHandler ProfileChanged;

	// TODO
	/*DeviceProfileCache(Ds4DeviceManager devices)
	{
		this.devices = devices;
	}*/

	void Load();

	/// <summary>
	/// Get a profile copy by name.
	/// </summary>
	/// <param name="profileName">The name of the profile to get.</param>
	/// <returns>A copy of the profile if found, else nullptr.</returns>
	std::unique_ptr<DeviceProfile> GetProfile(const std::string& profileName);

	/// <summary>
	/// Returns a copy of the cached settings for the specified MAC address.
	/// </summary>
	/// <param name="id">The MAC address of the device whose settings are to be copied.</param>
	/// <returns>The settings associated with the MAC address, or nullptr if none.</returns>
	std::unique_ptr<DeviceSettings> GetSettings(const std::string& id);

	/// <summary>
	/// Adds (or replaces) settings for the specified MAC address, then saves changes to disk.
	/// </summary>
	/// <param name="id">The MAC address of the device whose settings are being stored.</param>
	/// <param name="settings">The settings to be stored.</param>
	void SaveSettings(const std::string& id, const DeviceSettings& settings);

	/// <summary>
	/// Removes a profile from the profile cache.
	/// </summary>
	/// <param name="profile">The profile to be removed.</param>
	void RemoveProfile(const DeviceProfile& profile);

	/// <summary>
	/// Updates a profile and notifies all devices of the change.
	/// </summary>
	/// <param name="last">The profile to be replaced.</param>
	/// <param name="current">The new profile.</param>
	void UpdateProfile(const DeviceProfile& last, const DeviceProfile& current);

private:
	std::unique_ptr<DeviceProfile> FindProfile(const std::string& profileName);
	void LoadImpl();
	void OnLoaded() const;
	void OnProfileChanged(const std::string& oldName, const std::string& newName) const;
};
