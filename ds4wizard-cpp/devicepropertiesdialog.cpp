#include "stdafx.h"
#include "devicepropertiesdialog.h"

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
	// TODO: Profile
	// TODO: Use profile light
	// TODO: Automatic light color
	// TODO: Light color
	// TODO: Use profile idle settings
	// TODO: Disconnect on idle
	// TODO: Idle time
}
