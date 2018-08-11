#include "stdafx.h"
#include "DevicePropertiesDialog.h"
#include "ProfileEditorDialog.h"

// TODO: eventually allow configuring disconnected devices

using namespace std::chrono;

DevicePropertiesDialog::DevicePropertiesDialog(QWidget* parent, std::shared_ptr<Ds4Device> device_)
	: QDialog(parent),
	  device(std::move(device_))
{
	ui.setupUi(this);

	if (device != nullptr)
	{
		populateForm(this->device->settings);
		ui.tabReadout->setEnabled(true);

		connect(ui.tabWidget, &QTabWidget::currentChanged, this, &DevicePropertiesDialog::tabChanged);

		qRegisterMetaType<Ds4InputData>("Ds4InputData");
		connect(this, &DevicePropertiesDialog::readoutChanged, this, &DevicePropertiesDialog::updateReadout);
		connect(ui.buttonResetPeak, &QToolButton::clicked, this, &DevicePropertiesDialog::resetPeakLatency);
	}
	else
	{
		ui.tabReadout->setEnabled(false);
	}

	connect(ui.pushButton_Edit, &QPushButton::clicked, this, &DevicePropertiesDialog::profileEditClicked);
	connect(ui.buttonColor, &QPushButton::clicked, this, &DevicePropertiesDialog::colorEditClicked);

	// useful for quick debugging of the readout tab
	if (ui.tabWidget->currentIndex() == 1)
	{
		startReadout();
	}
}

DevicePropertiesDialog::~DevicePropertiesDialog()
{
	stopReadout();
}

void DevicePropertiesDialog::populateForm(const DeviceSettings& settings) const
{
	// TODO: Profile (get profile list!)

	ui.lineEdit_DeviceName->setText(QString::fromStdString(settings.name));

	ui.checkBox_UseProfileLight->setChecked(settings.useProfileLight);
	ui.checkBox_AutoLightColor->setChecked(settings.light.automaticColor);

	// TODO: Light color
	auto ds4color = settings.light.color;
	QColor color = QColor(ds4color.red, ds4color.green, ds4color.blue, 255);
	auto colorName = color.name();
	ui.buttonColor->setStyleSheet(QString("background-color: %1; border: none").arg(colorName));

	ui.checkBox_UseProfileIdle->setChecked(settings.useProfileIdle);
	ui.checkBox_IdleDisconnect->setChecked(settings.idle.disconnect);

	// TODO: Idle time
	// TODO: Idle fade
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

		std::this_thread::sleep_for(device->getLatencyAverage());
	}
}

void DevicePropertiesDialog::stopReadout()
{
	doReadout = false;

	if (readoutThread)
	{
		readoutThread->join();
		readoutThread = nullptr;
	}
}

void DevicePropertiesDialog::startReadout()
{
	if (doReadout)
	{
		return;
	}

	if (readoutThread)
	{
		stopReadout();
	}

	doReadout = true;
	readoutThread = std::make_unique<std::thread>(&DevicePropertiesDialog::readoutMethod, this);
}

void DevicePropertiesDialog::tabChanged(int index)
{
	if (!index)
	{
		stopReadout();
	}
	else
	{
		startReadout();
	}
}

void DevicePropertiesDialog::updateReadout(Ds4InputData data) const
{
	// Left stick
	ui.labelLX->setNum(data.leftStick.x);
	ui.labelLY->setNum(data.leftStick.y);

	// Right stick
	ui.labelRX->setNum(data.rightStick.x);
	ui.labelRY->setNum(data.rightStick.y);

	// Touch A
	ui.labelTouchAX->setNum(data.touchPoint1.x);
	ui.labelTouchAY->setNum(data.touchPoint1.y);

	// Touch B
	ui.labelTouchBX->setNum(data.touchPoint2.x);
	ui.labelTouchBY->setNum(data.touchPoint2.y);

	// Gyroscope
	ui.labelGyroX->setNum(data.gyro.x);
	ui.labelGyroY->setNum(data.gyro.y);
	ui.labelGyroZ->setNum(data.gyro.z);

	// Accelerometer
	ui.labelAccelX->setNum(data.accel.x);
	ui.labelAccelY->setNum(data.accel.y);
	ui.labelAccelZ->setNum(data.accel.z);

	// Triggers
	ui.labelTriggerL->setNum(data.leftTrigger);
	ui.sliderTriggerL->setValue(data.leftTrigger);

	ui.labelTriggerR->setNum(data.rightTrigger);
	ui.sliderTriggerR->setValue(data.rightTrigger);

	duration<double, std::milli> latencyNow;
	duration<double, std::milli> latencyAvg;
	duration<double, std::milli> latencyMax;

	{
		auto lock = device->lock();
		latencyNow = duration_cast<duration<double, std::milli>>(device->getLatency());
		latencyAvg = duration_cast<duration<double, std::milli>>(device->getLatencyAverage());
		latencyMax = duration_cast<duration<double, std::milli>>(device->getLatencyPeak());
	}

	ui.labelLatencyNow->setText(QString("%1 ms").arg(latencyNow.count()));
	ui.labelLatencyAverage->setText(QString("%1 ms").arg(latencyAvg.count()));
	ui.labelLatencyPeak->setText(QString("%1 ms").arg(latencyMax.count()));
}

void DevicePropertiesDialog::resetPeakLatency() const
{
	device->resetLatencyPeak();
}

void DevicePropertiesDialog::profileEditClicked(bool checked)
{
	// TODO
	auto dialog = new ProfileEditorDialog(this);
	dialog->exec();
	delete dialog;
}

void DevicePropertiesDialog::colorEditClicked(bool checked)
{
	// TODO
	auto dialog = new QColorDialog(this);
	dialog->exec();
	delete dialog;
}
