#include "pch.h"
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
}

std::optional<DeviceProfile> DeviceProfileCache::getProfile(const std::string& profileName)
{
	if (profileName.empty())
	{
		return std::nullopt;
	}

	LOCK(profiles);

	return findProfile(profileName);
}

std::optional<DeviceSettings> DeviceProfileCache::getSettings(const std::string& id)
{
	LOCK(deviceSettings);
	const auto it = deviceSettings.find(id);

	if (it == deviceSettings.end())
	{
		return std::nullopt;
	}

	return it->second;
}

void DeviceProfileCache::saveSettings(const std::string& id, const DeviceSettings& settings)
{
	LOCK(deviceSettings);

	const auto it = deviceSettings.find(id);

	// don't save if nothing has changed
	if (it != deviceSettings.end() && it->second == settings)
	{
		return;
	}

	const auto devicesFilePath = QString::fromStdString(Program::devicesFilePath());

	QDir devicesDir(devicesFilePath);
	devicesDir.cdUp();

	if (!devicesDir.exists())
	{
		devicesDir.mkpath(devicesDir.absolutePath());
	}

	QFile f(devicesFilePath);

	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		throw std::runtime_error("failed to open devices.json for writing");
	}

	deviceSettings[id] = settings;

	nlohmann::json json;

	for (auto& pair : deviceSettings)
	{
		json[pair.first] = pair.second.toJson();
	}
	
	f.write(QByteArray::fromStdString(json.dump(4)));
	f.close();
}

void DeviceProfileCache::removeProfile(const DeviceProfile& profile)
{
	{
		LOCK(profiles);

		const auto it = std::find(profiles.begin(), profiles.end(), profile);

		if (it != profiles.end())
		{
			profiles.erase(it);
		}
	}

	onProfileChanged(profile.name, std::string());

	const auto profilesPath = QString::fromStdString(Program::profilesPath());
	QDir profilesDir(profilesPath);

	if (!profilesDir.exists())
	{
		return;
	}

	QFile path = profilesDir.filePath(QString::fromStdString(profile.fileName()));

	if (path.exists())
	{
		path.remove();
	}
}

void DeviceProfileCache::updateProfile(const DeviceProfile& last, const DeviceProfile& current)
{
	if (current.name.empty())
	{
		throw std::runtime_error("profile name cannot be empty");
	}

	{
		LOCK(profiles);

		const auto it = std::find(profiles.begin(), profiles.end(), last);

		if (it != profiles.end())
		{
			profiles.erase(it);
		}

		profiles.push_back(current);
	}

	onProfileChanged(last.name, current.name);

	{
		LOCK(profiles);

		QDir profilesPath = QString::fromStdString(Program::profilesPath());

		if (!profilesPath.exists())
		{
			profilesPath.mkpath(profilesPath.absolutePath());
		}

		QString newPath = profilesPath.filePath(QString::fromStdString(current.fileName()));

		QFile f(newPath);

		if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			throw std::runtime_error(std::string("failed to open \"")
			                         + newPath.toStdString()
			                         + "\" for writing");
		}

		f.write(QByteArray::fromStdString(current.toJson().dump(4)));

		if (!last.name.empty() && !iequals(last.fileName(), current.fileName()))
		{
			QFile file = profilesPath.filePath(QString::fromStdString(last.fileName()));
			file.remove();
		}
	}
}

std::optional<DeviceProfile> DeviceProfileCache::findProfile(const std::string& profileName)
{
	/*return Profiles.FirstOrDefault(x => x.FileName.Equals(profileName, StringComparison.InvariantCultureIgnoreCase)
	                                   || x.Name.Equals(profileName, StringComparison.InvariantCultureIgnoreCase));*/

	LOCK(profiles);

	for (auto& profile : profiles)
	{
		if (iequals(profile.fileName(), profileName) || iequals(profile.name, profileName))
		{
			return profile;
		}
	}

	return std::nullopt;
}

void DeviceProfileCache::loadImpl()
{
	{
		LOCK(profiles);
		profiles.clear();

		QDir dir(QString::fromStdString(Program::profilesPath()));
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
				auto profile = JsonData::fromJson<DeviceProfile>(nlohmann::json::parse(data.toStdString()));

				profiles.push_back(std::move(profile));
			}
		}
	}

	{
		LOCK(deviceSettings);
		deviceSettings.clear();

		QFile devicesFile(QString::fromStdString(Program::devicesFilePath()));
		if (devicesFile.exists())
		{
			if (devicesFile.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				QString str = devicesFile.readAll();

				auto doc = nlohmann::json::parse(str.toStdString());
				
				for (auto& pair : doc.items())
				{
					deviceSettings[pair.key()] = JsonData::fromJson<DeviceSettings>(pair.value());
				}
			}
		}
	}
}

void DeviceProfileCache::onProfileChanged(const std::string& oldName, const std::string& newName)
{
	{
		LOCK(deviceManager);

		auto devices_lock = deviceManager->lockDevices();

		for (auto& pair : deviceManager->devices)
		{
			if (pair.second->settings.profile == oldName)
			{
				pair.second->onProfileChanged(newName);
			}
		}
	}

	//ProfileChanged?.Invoke(this, EventArgs.Empty);
}
