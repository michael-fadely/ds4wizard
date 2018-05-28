#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "program.h"

// TODO: single instance https://stackoverflow.com/a/26904110

int main(int argc, char** argv)
{
	QApplication application(argc, argv);

	Program::initialize();
	Program::loadSettings();

	MainWindow window;

	auto result = application.exec();

	Program::saveSettings();

	return result;
}
