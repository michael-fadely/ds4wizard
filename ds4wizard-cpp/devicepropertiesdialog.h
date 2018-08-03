#pragma once

#include <QDialog>
#include "ui_devicepropertiesdialog.h"

#include <memory>
#include <atomic>
#include <thread>

class DevicePropertiesDialog : public QDialog
{
	Q_OBJECT

private:
	std::wstring deviceKey;
	std::shared_ptr<Ds4Device> device;
	std::atomic_bool doReadout;
	std::unique_ptr<std::thread> readoutThread;

public:
	DevicePropertiesDialog(QWidget* parent, std::shared_ptr<Ds4Device> device_);
	~DevicePropertiesDialog();
	void populateForm(const DeviceSettings& settings);

private:
	Ui::DevicePropertiesDialog ui;
	void readoutMethod();

signals:
	void readoutChanged(Ds4InputData data);

private slots:
	void tabChanged(int index);
	void updateReadout(Ds4InputData data);
};
