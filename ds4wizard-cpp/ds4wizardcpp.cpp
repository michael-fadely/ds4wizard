#include "stdafx.h"
#include "ds4wizardcpp.h"
#include "devicepropertiesdialog.h"

ds4wizardcpp::ds4wizardcpp(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->supports_systray = QSystemTrayIcon::isSystemTrayAvailable();

	if (supports_systray)
	{
		tray_icon = std::make_unique<QSystemTrayIcon>(this);
		tray_icon->setIcon(QIcon(":/ds4wizardcpp/Resources/race_q00.ico"));
		tray_icon->show();
	}
}

void ds4wizardcpp::on_pushButton_DeviceProperties_clicked() const
{
	tray_icon->showMessage("hi", "this is a test");
	auto dialog = new DevicePropertiesDialog();
	dialog->exec();
	delete dialog;
}
