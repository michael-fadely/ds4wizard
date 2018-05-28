#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ds4wizardcpp.h"

class ds4wizardcpp : public QMainWindow
{
	Q_OBJECT

public:
	ds4wizardcpp(QWidget* parent = nullptr);
	~ds4wizardcpp();

	void changeEvent(QEvent* e) override;

private:
	Ui::ds4wizardcpp ui;
	bool supports_systray = false;
	QSystemTrayIcon* tray_icon = nullptr;

protected slots:
	void toggle_hide(QSystemTrayIcon::ActivationReason reason);
	void on_pushButton_DeviceProperties_clicked() const;
};
