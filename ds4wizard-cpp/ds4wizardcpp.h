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
	bool supportsSystemTray = false;
	QSystemTrayIcon* trayIcon = nullptr;

protected slots:
	void toggleHide(QSystemTrayIcon::ActivationReason reason);
	void devicePropertiesClicked() const;
};
