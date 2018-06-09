#include "stdafx.h"
#include "DeviceProfileCache.h"
#include "lock.h"

void DeviceProfileCache::Load()
{
	LoadImpl();
	OnLoaded();
}

std::optional<DeviceProfile> DeviceProfileCache::GetProfile(const std::string& profileName)
{
	if (profileName.empty())
	{
		return std::nullopt;
	}

	lock(profile);

	return FindProfile(profileName);
}

std::optional<DeviceSettings> DeviceProfileCache::GetSettings(const std::string& id)
{
	lock(deviceSettings);
	const auto it = deviceSettings.find(id);

	if (it == deviceSettings.end())
	{
		return std::nullopt;
	}

	return it->second;
}

void DeviceProfileCache::SaveSettings(const std::string& id, const DeviceSettings& settings)
{
	lock(deviceSettings);

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
		lock(profile);
		profiles.remove(profile);
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
		lock(profile);
		profiles.remove(last);
		profiles.push_back(current);
	}

	OnProfileChanged(last.Name, current.Name);

	{
		lock(profile);
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

std::optional<DeviceProfile> DeviceProfileCache::FindProfile(const std::string& profileName)
{
	/*return Profiles.FirstOrDefault(x => x.FileName.Equals(profileName, StringComparison.InvariantCultureIgnoreCase)
	                                   || x.Name.Equals(profileName, StringComparison.InvariantCultureIgnoreCase));*/

	for (auto& profile : profiles)
	{
		// TODO: case insensitive
		if (profile.FileName() == profileName || profile.Name == profileName)
		{
			return profile;
		}
	}

	return std::nullopt;
}

void DeviceProfileCache::LoadImpl()
{
	{
		lock(profile);
		profiles.clear();

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
		lock(deviceSettings);
		deviceSettings.clear();

		QFile devicesFile(Program::devicesFilePath());
		if (devicesFile.exists())
		{
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
		lock(devices);

		foreach(Ds4Device device in devices.Enumerate().Where(device = > device.Settings.Profile == oldName))
		{
			device.OnProfileChanged(newName);
		}
	}

	ProfileChanged?.Invoke(this, EventArgs.Empty);
#endif
}
