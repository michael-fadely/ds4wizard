#include "stdafx.h"
#include "program.h"

Settings Program::settings {};
Settings Program::lastSettings {};
QString Program::settingsDir_;
QString Program::settingsFile_;

const QString& Program::settingsDir()
{
	return settingsDir_;
}

const QString& Program::settingsFile()
{
	return settingsFile_;
}

void Program::initialize()
{
	QDir dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	dir.cdUp();

	const QString appDataLocation = dir.path();
	settingsDir_ = appDataLocation + "/ds4wizard";
	settingsFile_ = settingsDir_ + "/settings.json";
}

void Program::loadSettings()
{
	QFile file(settingsFile());

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "failed to open " << settingsFile() << "for reading.\n";
		return;
	}

	QString val = file.readAll();
	QJsonDocument configJson = QJsonDocument::fromJson(val.toUtf8());
	settings = JsonData::read<Settings>(configJson.object());
	lastSettings = settings;

	file.close();
}

void Program::saveSettings()
{
	if (lastSettings == settings)
	{
		return;
	}

	QFile file(settingsFile());

	if (!file.open(QIODevice::WriteOnly))
	{
		qDebug() << "failed to open " << settingsFile() << "for writing.\n";
		return;
	}

	QJsonObject obj;
	settings.write(obj);

	QJsonDocument doc;
	doc.setObject(obj);

	file.write(doc.toJson(QJsonDocument::Indented));
	file.close();

	lastSettings = settings;
}
