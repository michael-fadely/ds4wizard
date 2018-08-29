#pragma once

#include <QDialog>
#include <QColor>
#include "ui_DevicePropertiesDialog.h"

#include <memory>
#include <atomic>
#include <thread>

class DevicePropertiesDialog : public QDialog
{
	Q_OBJECT

private:
	DeviceSettings oldSettings;
	DeviceSettings newSettings;
	std::wstring deviceKey;
	std::shared_ptr<Ds4Device> device;
	std::atomic_bool doReadout;
	std::unique_ptr<std::thread> readoutThread;
	QColor lightColor;

public:
	DevicePropertiesDialog(QWidget* parent, std::shared_ptr<Ds4Device> device_);
	~DevicePropertiesDialog();
	void populateForm();

signals:
	void settingsChanged(const DeviceSettings& oldSettings, const DeviceSettings& newSettings);

private:
	Ui::DevicePropertiesDialog ui;
	void readoutMethod();
	void stopReadout();
	void startReadout();
	void applySettings();

signals:
	void readoutChanged(Ds4InputData data);

private slots:
	void tabChanged(int index);
	void updateReadout(Ds4InputData data) const;
	void resetPeakLatency() const;
	void profileEditClicked(bool checked);
	void colorEditClicked(bool checked);
	void buttonBoxAccepted();
	void applyButtonClicked(bool checked);
};
