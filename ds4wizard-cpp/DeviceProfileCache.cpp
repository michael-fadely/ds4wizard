#include "stdafx.h"
#include "DeviceProfileCache.h"
#include "lock.h"
#include <sstream>

void DeviceProfileCache::setDevices(const std::shared_ptr<Ds4DeviceManager>& deviceManager)
{
	this->deviceManager = deviceManager;
}

void DeviceProfileCache::load()
{
	loadImpl();
	OnLoaded();
}

std::optional<DeviceProfile> DeviceProfileCache::getProfile(const std::string& profileName)
{
	if (profileName.empty())
	{
		return std::nullopt;
	}

	lock(profiles);

	return findProfile(profileName);
}

std::optional<DeviceSettings> DeviceProfileCache::getSettings(const std::string& id)
{
	lock(deviceSettings);
	const auto it = deviceSettings.find(id);

	if (it == deviceSettings.end())
	{
		return std::nullopt;
	}

	return it->second;
}

void DeviceProfileCache::saveSettings(const std::string& id, const DeviceSettings& settings)
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

void DeviceProfileCache::removeProfile(const DeviceProfile& profile)
{
	{
		lock(profiles);
		profiles.remove(profile);
	}

	OnProfileChanged(profile.name, nullptr);

	if (filesystem::directory_exists(Program::profilesPath().toStdString()))
	{
		return;
	}

	const std::string path = filesystem::combine_path(Program::profilesPath().toStdString(), profile.fileName());

	if (filesystem::file_exists(path))
	{
		filesystem::remove(path);
	}
}

void DeviceProfileCache::updateProfile(const DeviceProfile& last, const DeviceProfile& current)
{
	{
		lock(profiles);
		profiles.remove(last);
		profiles.push_back(current);
	}

	OnProfileChanged(last.name, current.name);

	{
		lock(profiles);
		if (!filesystem::directory_exists(Program::profilesPath().toStdString()))
		{
			filesystem::create_directory(Program::profilesPath().toStdString());
		}

		std::string newPath = filesystem::combine_path(Program::profilesPath().toStdString(), current.fileName());

		QFile f(QString::fromStdString(newPath));

		if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			throw std::runtime_error(std::string("failed to open \"") + newPath + "\" for writing");
		}

		f.write(QJsonDocument(current.toJson()).toJson(QJsonDocument::Indented));

		// TODO: case insensitive
		if (last.fileName() != current.fileName())
		{
			filesystem::remove(filesystem::combine_path(Program::profilesPath().toStdString(), last.fileName()));
		}
	}
}

std::optional<DeviceProfile> DeviceProfileCache::findProfile(const std::string& profileName)
{
	/*return Profiles.FirstOrDefault(x => x.FileName.Equals(profileName, StringComparison.InvariantCultureIgnoreCase)
	                                   || x.Name.Equals(profileName, StringComparison.InvariantCultureIgnoreCase));*/

	lock(profiles);

	for (auto& profile : profiles)
	{
		// TODO: case insensitive
		if (profile.fileName() == profileName || profile.name == profileName)
		{
			return profile;
		}
	}

	return std::nullopt;
}

void DeviceProfileCache::loadImpl()
{
	{
		lock(profiles);
		profiles.clear();

		QDir dir(Program::profilesPath());
		if (dir.exists())
		{
			for (QString& fileName : dir.entryList({ "*.json" }, QDir::Files))
			{
				QString filePath = dir.filePath(fileName);
				QFile file(filePath);

				if (!file.open(QFile::ReadOnly))
				{
					std::stringstream msg;
					msg << "unable to open device profile " << filePath.toStdString() << " for reading";

					throw std::runtime_error(msg.str());
				}

				const QByteArray data = file.readAll();
				auto profile = JsonData::fromJson<DeviceProfile>(QJsonDocument::fromJson(data).object());

				profiles.push_back(std::move(profile));
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
	// TODO: deviceManager
#if 0
	{
		lock(deviceManager);

		foreach(Ds4Device device in deviceManager.Enumerate().Where(device = > device.Settings.Profile == oldName))
		{
			device.OnProfileChanged(newName);
		}
	}

	ProfileChanged?.Invoke(this, EventArgs.Empty);
#endif
}
