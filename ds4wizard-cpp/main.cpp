#include "stdafx.h"
#include "ds4wizardcpp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ds4wizardcpp w;
	w.show();
	return a.exec();
}
