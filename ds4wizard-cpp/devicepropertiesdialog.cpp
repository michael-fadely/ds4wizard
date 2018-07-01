#include "stdafx.h"
#include "devicepropertiesdialog.h"

// TODO: readout tab

DevicePropertiesDialog::DevicePropertiesDialog(QWidget* parent, std::wstring deviceKey_, std::shared_ptr<Ds4DeviceManager> manager_)
	: QDialog(parent),
	  deviceKey(std::move(deviceKey_)),
	  manager(std::move(manager_))
{
	ui.setupUi(this);

	{
		auto& devices_lock = manager->devices_lock;
		auto& devices = manager->devices;

		lock(devices);

		const auto it = devices.find(deviceKey);

		if (it != devices.end())
		{
			this->device = it->second;

			ui.lineEdit_DeviceName->setText(QString::fromStdString(this->device->name()));

			auto settings = this->device->settings;
			populateForm(settings);
		}
		else
		{
			ui.tabReadout->setEnabled(false);
			//populateForm(settings);
		}
	}
}

DevicePropertiesDialog::~DevicePropertiesDialog()
{
}

void DevicePropertiesDialog::populateForm(const DeviceSettings& settings)
{
	// TODO: Profile (get profile list!)

	ui.checkBox_UseProfileLight->setChecked(settings.useProfileLight);
	ui.checkBox_AutoLightColor->setChecked(settings.light.automaticColor);

	// TODO: Light color

	ui.checkBox_UseProfileIdle->setChecked(settings.useProfileIdle);
	ui.checkBox_IdleDisconnect->setChecked(settings.idle.disconnect);

	// TODO: Idle time
}
