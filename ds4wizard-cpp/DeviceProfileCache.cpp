#include "stdafx.h"
#include "DeviceProfileCache.h"

void DeviceProfileCache::Load()
{
	LoadImpl();
	OnLoaded();
}

bool DeviceProfileCache::GetProfile(const std::string& profileName, DeviceProfile& outProfile)
{
	if (profileName.empty())
	{
		return {};
	}

	std::lock_guard<std::mutex> guard(profile_lock);

	return FindProfile(profileName, outProfile);
}

bool DeviceProfileCache::GetSettings(const std::string& id, DeviceSettings& outSettings)
{
	std::lock_guard<std::mutex> guard(deviceSettings_lock);
	auto it = deviceSettings.find(id);

	if (it == deviceSettings.end())
	{
		return false;
	}

	outSettings = it->second;
	return true;
}

void DeviceProfileCache::SaveSettings(const std::string& id, const DeviceSettings& settings)
{
	std::lock_guard<std::mutex> guard(deviceSettings_lock);

	auto it = deviceSettings.find(id);

	if (it != deviceSettings.end() && it->second == settings)
	{
		return;
	}

	deviceSettings[id] = settings;
	File.WriteAllText(Program::devicesFilePath(), JsonConvert.SerializeObject(deviceSettings));
}

void DeviceProfileCache::RemoveProfile(const DeviceProfile& profile)
{
	{
		std::lock_guard<std::mutex> guard(profile_lock);
		Profiles.remove(profile);
	}

	OnProfileChanged(profile.Name, nullptr);

	if (!Directory.Exists(Program::profilesPath()))
	{
		return;
	}

	std::string path = Path.Combine(Program::profilesPath(), profile.FileName);

	if (File.Exists(path))
	{
		File.Delete(path);
	}
}

void DeviceProfileCache::UpdateProfile(const DeviceProfile& last, const DeviceProfile& current)
{
	{
		std::lock_guard<std::mutex> guard(profile_lock);
		Profiles.remove(last);
		Profiles.push_back(current);
	}

	OnProfileChanged(last.Name, current.Name);

	{
		std::lock_guard<std::mutex> guard(profile_lock);
		if (!Directory.Exists(Program::profilesPath()))
		{
			Directory.CreateDirectory(Program::profilesPath());
		}

		std::string newPath = Path.Combine(Program::profilesPath(), current.FileName);
		File.WriteAllText(newPath, JsonConvert.SerializeObject(current));

		if (!last.FileName.Equals(current.FileName, StringComparison.InvariantCultureIgnoreCase))
		{
			File.Delete(Path.Combine(Program::profilesPath(), last.FileName));
		}
	}
}

bool DeviceProfileCache::FindProfile(const std::string& profileName, DeviceProfile& outProfile)
{
	/*return Profiles.FirstOrDefault(x =  > x.FileName.Equals(profileName, StringComparison.InvariantCultureIgnoreCase)
	                                   || x.Name.Equals(profileName, StringComparison.InvariantCultureIgnoreCase));*/

	for (auto& profile : Profiles)
	{
		// TODO: case insensitive
		if (profile.FileName == profileName || profile.Name() == profileName)
		{
			outProfile = profile;
			return true;
		}
	}

	return false;
}

void DeviceProfileCache::LoadImpl()
{
	{
		std::lock_guard<std::mutex> guard(profile_lock);
		Profiles.clear();

		if (Directory.Exists(Program::profilesPath()))
		{
			foreach(std::string f in Directory.EnumerateFiles(Program::profilesPath()))
			{
				try
				{
					auto profile = JsonConvert.DeserializeObject<DeviceProfile>(File.ReadAllText(f));
					Profiles.Add(profile);
				}
				catch
				{
					// HACK: ignored
				}
			}
		}
	}

	{
		std::lock_guard<std::mutex> guard(deviceSettings_lock);
		deviceSettings.clear();

		if (File.Exists(Program::devicesFilePath()))
		{
			deviceSettings = JsonConvert.DeserializeObject<Dictionary<std::string, DeviceSettings>>(
			                                                                                        File.ReadAllText(Program::devicesFilePath())
			                                                                                       );

			foreach(DeviceSettings device in deviceSettings.Values.Where(device = > FindProfile(device.Profile) is nullptr))
			{
				device.Profile = nullptr;
			}
		}
	}
}

void DeviceProfileCache::OnLoaded()
{
	// TODO
	//Loaded?.Invoke(this, EventArgs.Empty);
}

void DeviceProfileCache::OnProfileChanged(const std::string& oldName, const std::string& newName)
{
	// TODO
#if 0
	{
		std::lock_guard<std::mutex> guard(devices_lock);

		foreach(Ds4Device device in devices.Enumerate().Where(device = > device.Settings.Profile == oldName))
		{
			device.OnProfileChanged(newName);
		}
	}

	ProfileChanged?.Invoke(this, EventArgs.Empty);
#endif
}
