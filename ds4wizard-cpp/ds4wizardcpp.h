#pragma once

#include <memory>
#include <QtWidgets/QMainWindow>
#include "ui_ds4wizardcpp.h"

class ds4wizardcpp : public QMainWindow
{
	Q_OBJECT

public:
	ds4wizardcpp(QWidget *parent = Q_NULLPTR);

private:
	Ui::ds4wizardcpp ui;
	bool supports_systray = false;
	std::unique_ptr<QSystemTrayIcon> tray_icon = nullptr;

protected slots:
	static void on_pushButton_DeviceProperties_clicked();
};
