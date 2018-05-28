#include "stdafx.h"
#include "ds4wizardcpp.h"
#include <QtWidgets/QApplication>
#include "program.h"

int main(int argc, char** argv)
{
	QApplication a(argc, argv);

	Program::initialize();
	Program::loadSettings();

	ds4wizardcpp w;
	return a.exec();
}
