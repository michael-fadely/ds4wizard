#pragma once
#include "Settings.h"

class Program
{
	static Settings lastSettings;
	static QString settingsDir_;
	static QString settingsFile_;

public:
	static Settings settings;

	static const QString& settingsDir();
	static const QString& settingsFile();

	static void initialize();
	static void loadSettings();
	static void saveSettings();
};
