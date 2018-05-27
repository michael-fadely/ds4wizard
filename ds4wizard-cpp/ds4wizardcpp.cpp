#include "stdafx.h"
#include "ds4wizardcpp.h"
#include "devicepropertiesdialog.h"

ds4wizardcpp::ds4wizardcpp(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

void ds4wizardcpp::on_pushButton_DeviceProperties_clicked()
{
	auto dialog = new DevicePropertiesDialog();
	dialog->exec();
	delete dialog;
}
