#include "pch.h"
#include "program.h"
#include "DeviceProfileCache.h"

BOOL IsElevated()
{
	BOOL fRet     = FALSE;
	HANDLE hToken = nullptr;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		TOKEN_ELEVATION elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);

		if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize))
		{
			fRet = elevation.TokenIsElevated;
		}
	}

	if (hToken)
	{
		CloseHandle(hToken);
	}

	return fRet;
}

DeviceProfileCache Program::profileCache {};
Settings Program::settings {};
Settings Program::lastSettings {};
QString Program::settingsPath;
QString Program::settingsFilePath;
std::string Program::profilesPath_;
std::string Program::devicesFilePath_;

vigem::Driver Program::driver;

const std::string& Program::profilesPath()
{
	return profilesPath_;
}

const std::string& Program::devicesFilePath()
{
	return devicesFilePath_;
}

bool Program::isElevated()
{
	return isElevated_;
}

void Program::initialize()
{
	// TODO: check for and display error opening ViGEm driver handle
	driver.open();

	// HACK: don't do this?
	QDir dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	dir.cdUp();

	const QString appDataLocation = dir.path();

	settingsPath     = appDataLocation + "/ds4wizard";
	settingsFilePath = settingsPath + "/settings.json";
	profilesPath_    = (settingsPath + "/profiles").toStdString();
	devicesFilePath_ = (settingsPath + "/devices.json").toStdString();

	isElevated_ = IsElevated() == TRUE;
}

void Program::loadSettings()
{
	QDir settingsDir(settingsPath);

	if (!settingsDir.exists())
	{
		qDebug() << "no settings to load";
		return;
	}

	QFile file(settingsFilePath);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "failed to open " << settingsFilePath << "for reading.\n";
		return;
	}

	QString val     = file.readAll();
	const auto json = nlohmann::json::parse(val.toStdString());
	settings        = JsonData::fromJson<Settings>(json);
	lastSettings    = settings;

	file.close();
}

void Program::saveSettings()
{
	if (lastSettings == settings)
	{
		return;
	}

	QDir settingsDir(settingsPath);

	if (!settingsDir.exists())
	{
		// there's evidently a good reason for mkpath
		// to be non-static, but it still feels weird.
		settingsDir.mkpath(settingsPath);
	}

	QFile file(settingsFilePath);

	if (!file.open(QIODevice::WriteOnly))
	{
		qDebug() << "failed to open " << settingsFilePath << "for writing.\n";
		return;
	}

	nlohmann::json obj;
	settings.writeJson(obj);

	file.write(QByteArray::fromStdString(obj.dump(4)));
	file.close();

	lastSettings = settings;
}
