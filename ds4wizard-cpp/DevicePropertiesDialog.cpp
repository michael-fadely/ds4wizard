#include "stdafx.h"
#include "DevicePropertiesDialog.h"
#include "ProfileEditorDialog.h"

// TODO: eventually allow configuring disconnected devices

using namespace std::chrono;

QColor toQt(const Ds4Color ds4Color)
{
	return QColor(ds4Color.red, ds4Color.green, ds4Color.blue, 255);
}

Ds4Color toDs4(const QColor& color)
{
	return Ds4Color(color.red(), color.green(), color.blue());
}

DevicePropertiesDialog::DevicePropertiesDialog(QWidget* parent, std::shared_ptr<Ds4Device> device_)
	: QDialog(parent),
	  device(std::move(device_))
{
	ui.setupUi(this);

	if (device != nullptr)
	{
		oldSettings = this->device->settings;
		newSettings = oldSettings;

		populateForm();
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
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &DevicePropertiesDialog::buttonBoxAccepted);

	auto applyButton = ui.buttonBox->button(QDialogButtonBox::Apply);
	connect(applyButton, &QPushButton::clicked, this, &DevicePropertiesDialog::applyButtonClicked);

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

void DevicePropertiesDialog::setColorPickerColor()
{
	auto colorName = this->lightColor.name();
	ui.buttonColor->setStyleSheet(QString("background-color: %1; border: none").arg(colorName));
}

void DevicePropertiesDialog::populateForm()
{
	// TODO: Profile (get profile list!)

	ui.lineEdit_DeviceName->setText(QString::fromStdString(oldSettings.name));

	ui.checkBox_UseProfileLight->setChecked(oldSettings.useProfileLight);
	ui.checkBox_AutoLightColor->setChecked(oldSettings.light.automaticColor);

	this->lightColor = toQt(oldSettings.light.color);
	setColorPickerColor();

	ui.checkBox_UseProfileIdle->setChecked(oldSettings.useProfileIdle);
	ui.checkBox_IdleDisconnect->setChecked(oldSettings.idle.disconnect);

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

void DevicePropertiesDialog::applySettings()
{
	newSettings.name = ui.lineEdit_DeviceName->text().toStdString();

	newSettings.useProfileLight      = ui.checkBox_UseProfileLight->isChecked();
	newSettings.light.automaticColor = ui.checkBox_AutoLightColor->isChecked();
	newSettings.useProfileIdle       = ui.checkBox_UseProfileIdle->isChecked();

	newSettings.light.color = toDs4(this->lightColor);

	// TODO: Idle time
	// TODO: Idle fade

	if (newSettings == oldSettings)
	{
		return;
	}

	emit settingsChanged(oldSettings, newSettings);

	oldSettings = newSettings;
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

void DevicePropertiesDialog::profileEditClicked(bool /*checked*/)
{
	// TODO
	auto dialog = new ProfileEditorDialog(this);
	dialog->exec();
	delete dialog;
}

void DevicePropertiesDialog::colorEditClicked(bool /*checked*/)
{
	auto dialog = new QColorDialog(this);
	dialog->setCurrentColor(toQt(oldSettings.light.color));

	if (dialog->exec() == QDialog::Accepted)
	{
		this->lightColor = dialog->currentColor();
		setColorPickerColor();
	}

	delete dialog;
}

void DevicePropertiesDialog::buttonBoxAccepted()
{
	applySettings();
	close();
}

void DevicePropertiesDialog::applyButtonClicked(bool /*checked*/)
{
	applySettings();
}
