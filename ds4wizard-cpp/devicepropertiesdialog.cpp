#include "stdafx.h"
#include "devicepropertiesdialog.h"

// TODO: readout tab
// TODO: eventually allow configuring disconnected devices

DevicePropertiesDialog::DevicePropertiesDialog(QWidget* parent, std::shared_ptr<Ds4Device> device_)
	: QDialog(parent),
	  device(std::move(device_))
{
	ui.setupUi(this);
	if (device != nullptr)
	{
		populateForm(this->device->settings);
		ui.tabReadout->setEnabled(true);
	}
	else
	{
		ui.tabReadout->setEnabled(false);
	}
}

DevicePropertiesDialog::~DevicePropertiesDialog()
{
}

void DevicePropertiesDialog::populateForm(const DeviceSettings& settings)
{
	// TODO: Profile (get profile list!)

	ui.lineEdit_DeviceName->setText(QString::fromStdString(settings.name));

	ui.checkBox_UseProfileLight->setChecked(settings.useProfileLight);
	ui.checkBox_AutoLightColor->setChecked(settings.light.automaticColor);

	// TODO: Light color

	ui.checkBox_UseProfileIdle->setChecked(settings.useProfileIdle);
	ui.checkBox_IdleDisconnect->setChecked(settings.idle.disconnect);

	// TODO: Idle time
}
