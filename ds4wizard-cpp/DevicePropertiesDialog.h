#pragma once

#include <QDialog>
#include <QColor>
#include "ui_DevicePropertiesDialog.h"

#include <atomic>
#include <memory>
#include <thread>

class DevicePropertiesDialog : public QDialog
{
	Q_OBJECT

private:
	DeviceSettings oldSettings;
	DeviceSettings newSettings;
	std::wstring deviceKey;
	std::shared_ptr<Ds4Device> device;
	std::atomic_bool doReadout = false;
	std::unique_ptr<std::thread> readoutThread;
	QColor lightColor;
	int lastUnit = 0;

public:
	DevicePropertiesDialog(QWidget* parent, std::shared_ptr<Ds4Device> device_);
	~DevicePropertiesDialog();
	void setColorPickerColor() const;
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
	void readoutChanged(Ds4Buttons_t heldButtons, Ds4InputData data);

private slots:
	void tabChanged(int index);
	void updateReadout(Ds4Buttons_t heldButtons, Ds4InputData data) const;
	void resetPeakLatency() const;
	void profileEditClicked(bool checked);
	void colorEditClicked(bool checked);
	void buttonBoxAccepted();
	void applyButtonClicked(bool checked);
	std::chrono::high_resolution_clock::duration getGuiIdleTime() const;
	void timeUnitChanged(int index);
};
