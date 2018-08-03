#include "stdafx.h"
#include "devicepropertiesdialog.h"

// TODO: finish readout tab
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

		connect(ui.tabWidget, SIGNAL(currentChanged(int)),
				this, SLOT(tabChanged(int)));

		qRegisterMetaType<Ds4InputData>("Ds4InputData");

		connect(this, SIGNAL(readoutChanged(Ds4InputData)), this, SLOT(updateReadout(Ds4InputData)));
	}
	else
	{
		ui.tabReadout->setEnabled(false);
	}
}

DevicePropertiesDialog::~DevicePropertiesDialog()
{
	if (readoutThread)
	{
		doReadout = false;
		readoutThread->join();
		readoutThread = nullptr;
	}
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

void DevicePropertiesDialog::readoutMethod()
{
	Ds4InputData last {};
	while (doReadout)
	{
		auto data = device->input.data;

		if (data != last)
		{
			last = data;
			emit readoutChanged(data);
		}

		std::this_thread::sleep_for(device->getLatency());
	}
}

void DevicePropertiesDialog::tabChanged(int index)
{
	if (!index)
	{
		doReadout = false;
		readoutThread->join();
		readoutThread = nullptr;
	}
	else
	{
		doReadout = true;
		readoutThread = std::make_unique<std::thread>(&DevicePropertiesDialog::readoutMethod, this);
	}
}

void DevicePropertiesDialog::updateReadout(Ds4InputData data)
{
	ui.labelLX->setNum(data.leftStick.x);
	ui.labelLY->setNum(data.leftStick.y);
	ui.labelRX->setNum(data.rightStick.x);
	ui.labelRY->setNum(data.rightStick.y);
}
