#pragma once

#include <string>
#include <list>
#include <unordered_map>

#include "DeviceSettings.h"
#include "DeviceProfile.h"

class DeviceProfileCache
{
#if 0
	Ds4DeviceManager devices;
	std::unordered_map<std::string, DeviceSettings> deviceSettings;

public:
	std::list<DeviceProfile> Profiles;

	//event EventHandler Loaded;
	//event EventHandler ProfileChanged;

	DeviceProfileCache(Ds4DeviceManager devices)
	{
		this.devices = devices;
	}

	void Load()
	{
		LoadImpl();
		OnLoaded();
	}

	/// <summary>
	/// Get a profile copy by name.
	/// </summary>
	/// <param name="profileName">The name of the profile to get.</param>
	/// <returns>A copy of the profile if found, else null.</returns>
	DeviceProfile GetProfile(string profileName)
	{
		if (string.IsNullOrEmpty(profileName))
		{
			return null;
		}

		lock(Profiles)
		{
			DeviceProfile result = FindProfile(profileName);
			return result == null ? null : new DeviceProfile(result);
		}
	}

	/// <summary>
	/// Returns a copy of the cached settings for the specified MAC address.
	/// </summary>
	/// <param name="id">The MAC address of the device whose settings are to be copied.</param>
	/// <returns>The settings associated with the MAC address, or null if none.</returns>
	DeviceSettings GetSettings(string id)
	{
		lock(deviceSettings)
		{
			return deviceSettings.TryGetValue(id, out DeviceSettings o) ? new DeviceSettings(o) : null;
		}
	}

	/// <summary>
	/// Adds (or replaces) settings for the specified MAC address, then saves changes to disk.
	/// </summary>
	/// <param name="id">The MAC address of the device whose settings are being stored.</param>
	/// <param name="settings">The settings to be stored.</param>
	void SaveSettings(string id, DeviceSettings settings)
	{
		lock(deviceSettings)
		{
			deviceSettings.TryGetValue(id, out DeviceSettings last);
			deviceSettings[id] = new DeviceSettings(settings);

			if (last != null && last.Equals(settings))
			{
				return;
			}

			File.WriteAllText(Program.DevicesFilePath, JsonConvert.SerializeObject(deviceSettings));
		}
	}

	/// <summary>
	/// Removes a profile from the profile cache.
	/// </summary>
	/// <param name="profile">The profile to be removed.</param>
	void RemoveProfile(DeviceProfile profile)
	{
		lock(Profiles)
		{
			Profiles.Remove(profile);
		}

		OnProfileChanged(profile.Name, null);

		if (!Directory.Exists(Program.ProfilesPath))
		{
			return;
		}

		string path = Path.Combine(Program.ProfilesPath, profile.FileName);

		if (File.Exists(path))
		{
			File.Delete(path);
		}
	}

	/// <summary>
	/// Updates a profile and notifies all devices of the change.
	/// </summary>
	/// <param name="last">The profile to be replaced.</param>
	/// <param name="current">The new profile.</param>
	void UpdateProfile(DeviceProfile last, DeviceProfile current)
	{
		lock(Profiles)
		{
			Profiles.Remove(last);
			Profiles.Add(current);
		}

		OnProfileChanged(last.Name, current.Name);

		lock(Profiles)
		{
			if (!Directory.Exists(Program.ProfilesPath))
			{
				Directory.CreateDirectory(Program.ProfilesPath);
			}

			string newPath = Path.Combine(Program.ProfilesPath, current.FileName);
			File.WriteAllText(newPath, JsonConvert.SerializeObject(current));

			if (!last.FileName.Equals(current.FileName, StringComparison.InvariantCultureIgnoreCase))
			{
				File.Delete(Path.Combine(Program.ProfilesPath, last.FileName));
			}
		}
	}

private:
	DeviceProfile FindProfile(string profileName)
	{
		return Profiles.FirstOrDefault(x = > x.FileName.Equals(profileName, StringComparison.InvariantCultureIgnoreCase)
			|| x.Name.Equals(profileName, StringComparison.InvariantCultureIgnoreCase));
	}

	void LoadImpl()
	{
		lock(Profiles)
		{
			Profiles.Clear();

			if (Directory.Exists(Program.ProfilesPath))
			{
				foreach(string f in Directory.EnumerateFiles(Program.ProfilesPath))
				{
					try
					{
						var profile = JsonConvert.DeserializeObject<DeviceProfile>(File.ReadAllText(f));
						Profiles.Add(profile);
					}
					catch
					{
						// HACK: ignored
					}
				}
			}
		}

		lock(deviceSettings)
		{
			deviceSettings.Clear();

			if (File.Exists(Program.DevicesFilePath))
			{
				deviceSettings = JsonConvert.DeserializeObject<Dictionary<string, DeviceSettings>>(
					File.ReadAllText(Program.DevicesFilePath)
					);

				foreach(DeviceSettings device in deviceSettings.Values.Where(device = > FindProfile(device.Profile) is null))
				{
					device.Profile = null;
				}
			}
		}
	}

	void OnLoaded()
	{
		Loaded ? .Invoke(this, EventArgs.Empty);
	}

	void OnProfileChanged(string oldName, string newName)
	{
		lock(devices)
		{
			foreach(Ds4Device device in devices.Enumerate().Where(device = > device.Settings.Profile == oldName))
			{
				device.OnProfileChanged(newName);
			}
		}

		ProfileChanged ? .Invoke(this, EventArgs.Empty);
	}
#endif
};
