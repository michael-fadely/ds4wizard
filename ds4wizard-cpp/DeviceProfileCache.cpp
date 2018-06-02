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
	const auto it = deviceSettings.find(id);

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

	const auto it = deviceSettings.find(id);

	if (it != deviceSettings.end() && it->second == settings)
	{
		return;
	}

	QFile f(Program::devicesFilePath());

	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		throw std::runtime_error("failed to open devices.json for writing");
	}

	QJsonObject obj;

	for (auto& pair : deviceSettings)
	{
		obj[pair.first.c_str()] = pair.second.toJson();
	}

	auto doc = QJsonDocument(obj);

	f.write(doc.toJson(QJsonDocument::Indented));
	f.close();
	deviceSettings[id] = settings;
}

void DeviceProfileCache::RemoveProfile(const DeviceProfile& profile)
{
	{
		std::lock_guard<std::mutex> guard(profile_lock);
		Profiles.remove(profile);
	}

	OnProfileChanged(profile.Name, nullptr);

	if (filesystem::directory_exists(Program::profilesPath().toStdString()))
	{
		return;
	}

	const std::string path = filesystem::combine_path(Program::profilesPath().toStdString(), profile.FileName());

	if (filesystem::file_exists(path))
	{
		filesystem::remove(path);
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
		if (!filesystem::directory_exists(Program::profilesPath().toStdString()))
		{
			filesystem::create_directory(Program::profilesPath().toStdString());
		}

		std::string newPath = filesystem::combine_path(Program::profilesPath().toStdString(), current.FileName());

		QFile f(QString::fromStdString(newPath));

		if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			throw std::runtime_error(std::string("failed to open \"") + newPath + "\" for writing");
		}

		f.write(QJsonDocument(current.toJson()).toJson(QJsonDocument::Indented));

		// TODO: case insensitive
		if (last.FileName() != current.FileName())
		{
			filesystem::remove(filesystem::combine_path(Program::profilesPath().toStdString(), last.FileName()));
		}
	}
}

bool DeviceProfileCache::FindProfile(const std::string& profileName, DeviceProfile& outProfile)
{
	/*return Profiles.FirstOrDefault(x => x.FileName.Equals(profileName, StringComparison.InvariantCultureIgnoreCase)
	                                   || x.Name.Equals(profileName, StringComparison.InvariantCultureIgnoreCase));*/

	for (auto& profile : Profiles)
	{
		// TODO: case insensitive
		if (profile.FileName() == profileName || profile.Name == profileName)
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

		//if (filesystem::directory_exists(Program::profilesPath().toStdString()))
		//{
		//	foreach(std::string f in Directory.EnumerateFiles(Program::profilesPath()))
		//	{
		//		try
		//		{
		//			auto profile = JsonConvert.DeserializeObject<DeviceProfile>(File.ReadAllText(f));
		//			Profiles.Add(profile);
		//		}
		//		catch
		//		{
		//			// HACK: ignored
		//		}
		//	}
		//}

		QDir dir(Program::profilesPath());
		if (dir.exists())
		{
			for (auto& f : dir.entryList({ "*.json" }, QDir::Files))
			{
				// TODO: DeviceProfile
				qDebug() << "PROFILE " + f;
			}
		}
	}

	{
		std::lock_guard<std::mutex> guard(deviceSettings_lock);
		deviceSettings.clear();

		QFile devicesFile(Program::devicesFilePath());
		if (devicesFile.exists())
		{
			//deviceSettings = JsonConvert.DeserializeObject<Dictionary<std::string, DeviceSettings>>(
			//                                                                                        File.ReadAllText(Program::devicesFilePath())
			//                                                                                       );

			//foreach(DeviceSettings device in deviceSettings.Values.Where(device = > FindProfile(device.Profile) is nullptr))
			//{
			//	device.Profile = nullptr;
			//}

			if (devicesFile.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				QString str = devicesFile.readAll();

				auto doc = QJsonDocument::fromJson(str.toUtf8());
				QJsonObject object = doc.object();
				
				for (auto& key : object.keys())
				{
					deviceSettings[key.toStdString()] = JsonData::fromJson<DeviceSettings>(object[key].toObject());
				}
			}
		}
	}
}

void DeviceProfileCache::OnLoaded() const
{
	// TODO
	//Loaded?.Invoke(this, EventArgs.Empty);
}

void DeviceProfileCache::OnProfileChanged(const std::string& oldName, const std::string& newName) const
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
