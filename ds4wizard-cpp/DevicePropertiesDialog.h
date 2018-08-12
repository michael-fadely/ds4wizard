#pragma once

#include <QDialog>
#include "ui_DevicePropertiesDialog.h"

#include <memory>
#include <atomic>
#include <thread>

class DevicePropertiesDialog : public QDialog
{
	Q_OBJECT

private:
	DeviceSettings deviceSettings;
	std::wstring deviceKey;
	std::shared_ptr<Ds4Device> device;
	std::atomic_bool doReadout;
	std::unique_ptr<std::thread> readoutThread;

public:
	DevicePropertiesDialog(QWidget* parent, std::shared_ptr<Ds4Device> device_);
	~DevicePropertiesDialog();
	void populateForm() const;

private:
	Ui::DevicePropertiesDialog ui;
	void readoutMethod();
	void stopReadout();
	void startReadout();

signals:
	void readoutChanged(Ds4InputData data);

private slots:
	void tabChanged(int index);
	void updateReadout(Ds4InputData data) const;
	void resetPeakLatency() const;
	void profileEditClicked(bool checked);
	void colorEditClicked(bool checked);
};
