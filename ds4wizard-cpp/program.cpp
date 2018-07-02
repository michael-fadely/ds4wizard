#include "stdafx.h"
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
QString Program::settingsPath_;
QString Program::settingsFilePath_;
QString Program::profilesPath_;
QString Program::devicesFilePath_;

const QString& Program::settingsPath()
{
	return settingsPath_;
}

const QString& Program::settingsFilePath()
{
	return settingsFilePath_;
}

const QString& Program::profilesPath()
{
	return profilesPath_;
}

const QString& Program::devicesFilePath()
{
	return devicesFilePath_;
}

bool Program::isElevated()
{
	return isElevated_;
}

void Program::initialize()
{
	// HACK: don't do this?
	QDir dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	dir.cdUp();

	const QString appDataLocation = dir.path();

	settingsPath_     = appDataLocation + "/ds4wizard";
	settingsFilePath_ = settingsPath_ + "/settings.json";
	profilesPath_     = settingsPath_ + "/profiles";
	devicesFilePath_  = settingsPath_ + "/devices.json";

	isElevated_ = !!IsElevated();
}

void Program::loadSettings()
{
	QFile file(settingsFilePath());

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "failed to open " << settingsFilePath() << "for reading.\n";
		return;
	}

	QString val = file.readAll();
	auto json = nlohmann::json::parse(val.toStdString());
	settings = JsonData::fromJson<Settings>(json);
	lastSettings = settings;

	file.close();
}

void Program::saveSettings()
{
	if (lastSettings == settings)
	{
		return;
	}

	QFile file(settingsFilePath());

	if (!file.open(QIODevice::WriteOnly))
	{
		qDebug() << "failed to open " << settingsFilePath() << "for writing.\n";
		return;
	}

	nlohmann::json obj;
	settings.writeJson(obj);

	file.write(QByteArray::fromStdString(obj.dump(4)));
	file.close();

	lastSettings = settings;
}
