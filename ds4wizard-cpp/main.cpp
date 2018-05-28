#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "program.h"

int main(int argc, char** argv)
{
	QApplication application(argc, argv);

	Program::initialize();
	Program::loadSettings();

	MainWindow window;
	return application.exec();
}
