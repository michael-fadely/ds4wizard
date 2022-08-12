#include "pch.h"
#include "DevicePropertiesDialog.h"
#include "ProfileEditorDialog.h"
#include "DeviceProfileModel.h"

#include <chrono>

// TODO: eventually allow configuring disconnected devices
// TODO: only allow minutes or seconds & minutes, drop hours?

using namespace std::chrono;

QColor toQt(const Ds4Color& ds4Color)
{
	return QColor(ds4Color.red, ds4Color.green, ds4Color.blue, 255);
}

Ds4Color toDs4(const QColor& color)
{
	return Ds4Color(static_cast<uint8_t>(color.red()), static_cast<uint8_t>(color.green()), static_cast<uint8_t>(color.blue()));
}

DevicePropertiesDialog::DevicePropertiesDialog(QWidget* parent, std::shared_ptr<Ds4Device> device_)
	: QDialog(parent),
	  device(std::move(device_)),
	  ui()
{
	ui.setupUi(this);
	ui.comboBox_Profile->setModel(new DeviceProfileItemModel(ui.comboBox_Profile, Program::profileCache, true));

	connect(ui.pushButton_Edit, &QPushButton::clicked, this, &DevicePropertiesDialog::profileEditClicked);
	connect(ui.buttonColor, &QPushButton::clicked, this, &DevicePropertiesDialog::colorEditClicked);
	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &DevicePropertiesDialog::buttonBoxAccepted);

	const auto applyButton = ui.buttonBox->button(QDialogButtonBox::Apply);
	connect(applyButton, &QPushButton::clicked, this, &DevicePropertiesDialog::applyButtonClicked);

	connect(ui.comboBox_IdleUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(timeUnitChanged(int)));

	if (device != nullptr)
	{
		oldSettings = this->device->settings;
		newSettings = oldSettings;

		populateForm();
		ui.tabReadout->setEnabled(true);

		connect(ui.tabWidget, &QTabWidget::currentChanged, this, &DevicePropertiesDialog::tabChanged);

		qRegisterMetaType<Ds4InputData>("Ds4InputData");
		qRegisterMetaType<Ds4Buttons_t>("Ds4Buttons_t");
		connect(this, &DevicePropertiesDialog::readoutChanged, this, &DevicePropertiesDialog::updateReadout);
		connect(ui.buttonResetPeak, &QToolButton::clicked, this, &DevicePropertiesDialog::resetPeakLatency);
	}
	else
	{
		ui.tabReadout->setEnabled(false);
	}

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

void DevicePropertiesDialog::setColorPickerColor() const
{
	const QString colorName = this->lightColor.name();
	ui.buttonColor->setStyleSheet(QString("background-color: %1; border: none").arg(colorName));
}

void DevicePropertiesDialog::populateForm()
{
	if (!oldSettings.profile.empty())
	{
		ui.comboBox_Profile->setCurrentText(QString::fromStdString(oldSettings.profile));
	}

	ui.lineEdit_DeviceName->setText(QString::fromStdString(oldSettings.name));

	ui.checkBox_UseProfileLight->setChecked(oldSettings.useProfileLight);
	ui.checkBox_AutoLightColor->setChecked(oldSettings.light.automaticColor);

	this->lightColor = toQt(oldSettings.light.color);
	setColorPickerColor();

	ui.checkBox_UseProfileIdle->setChecked(oldSettings.useProfileIdle);
	ui.checkBox_IdleDisconnect->setChecked(oldSettings.idle.disconnect);
	ui.checkBox_IdleFade->setChecked(oldSettings.light.idleFade);

	ui.spinBox_IdleTime->setValue(static_cast<double>(oldSettings.idle.timeout.count()));

	if (oldSettings.idle.timeout >= 1h)
	{
		ui.comboBox_IdleUnit->setCurrentIndex(2);
	}
	else if (oldSettings.idle.timeout >= 1min)
	{
		ui.comboBox_IdleUnit->setCurrentIndex(1);
	}
}

void DevicePropertiesDialog::readoutMethod()
{
	Ds4InputData last {};

	while (doReadout)
	{
		Ds4InputData data = device->input.data;
		Ds4Buttons_t heldButtons = device->input.heldButtons;

		if (data != last)
		{
			last = data;
			emit readoutChanged(heldButtons, data);
		}

		auto averageRead = device->getReadLatency();
		std::this_thread::sleep_for(averageRead.average() + 1ms);
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
	newSettings.light.idleFade       = ui.checkBox_IdleFade->isChecked();

	newSettings.light.color = toDs4(this->lightColor);

	newSettings.idle.timeout = duration_cast<seconds>(getGuiIdleTime());

	if (ui.comboBox_Profile->currentIndex() > 0)
	{
		std::string str = ui.comboBox_Profile->currentText().toStdString();
		std::optional<DeviceProfile> profile = Program::profileCache.getProfile(str);

		if (!profile.has_value())
		{
			throw std::runtime_error("invalid profile somehow");
		}

		newSettings.profile = profile->name;
	}
	else
	{
		newSettings.profile = std::string();
	}

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

void DevicePropertiesDialog::updateReadout(Ds4Buttons_t heldButtons, Ds4InputData data) const
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

	ui.labelRawButtons->setText(QString::fromStdString(fmt::format("{0:08X}", data.activeButtons & Ds4ButtonsRaw::mask)));
	ui.labelButtons->setText(QString::fromStdString(fmt::format("{0:08X}", heldButtons)));

	// TODO: write latency
	auto latency = device->getReadLatency();

	auto latencyNow = duration_cast<duration<double, std::milli>>(latency.lastValue());
	auto latencyAvg = duration_cast<duration<double, std::milli>>(latency.average());
	auto latencyMax = duration_cast<duration<double, std::milli>>(latency.peak());

	ui.labelLatencyNow->setText(QString("%1 ms").arg(latencyNow.count()));
	ui.labelLatencyAverage->setText(QString("%1 ms").arg(latencyAvg.count()));
	ui.labelLatencyPeak->setText(QString("%1 ms").arg(latencyMax.count()));
}

void DevicePropertiesDialog::resetPeakLatency() const
{
	// TODO: write latency
	device->resetReadLatencyPeak();
}

void DevicePropertiesDialog::profileEditClicked(bool /*checked*/)
{
	// TODO: implement actual functionality
	const int index = ui.comboBox_Profile->currentIndex();

	if (index >= 0)
	{
		const auto profileItems = dynamic_cast<DeviceProfileItemModel*>(ui.comboBox_Profile->model());
		const auto model = std::make_unique<DeviceProfileModel>(this, profileItems->getProfile(index));
		auto dialog = new ProfileEditorDialog(model.get(), this);
		dialog->exec();
		delete dialog;
	}
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

enum
{
	unit_seconds,
	unit_minutes,
	unit_hours
};

high_resolution_clock::duration DevicePropertiesDialog::getGuiIdleTime() const
{
	switch (lastUnit)
	{
		case unit_seconds:
			return duration_cast<high_resolution_clock::duration>(duration<double>(ui.spinBox_IdleTime->value()));

		case unit_minutes:
			return duration_cast<high_resolution_clock::duration>(duration<double, std::ratio<60>>(ui.spinBox_IdleTime->value()));

		case unit_hours:
			return duration_cast<high_resolution_clock::duration>(duration<double, std::ratio<3600>>(ui.spinBox_IdleTime->value()));

		default:
			throw std::out_of_range("invalid time unit");
	}
}

void DevicePropertiesDialog::timeUnitChanged(int index)
{
	const auto dur = getGuiIdleTime();

	switch (index)
	{
		case unit_seconds:
			ui.spinBox_IdleTime->setValue(duration_cast<duration<double>>(dur).count());
			break;

		case unit_minutes:
			ui.spinBox_IdleTime->setValue(duration_cast<duration<double, std::ratio<60>>>(dur).count());
			break;

		case unit_hours:
			ui.spinBox_IdleTime->setValue(duration_cast<duration<double, std::ratio<3600>>>(dur).count());
			break;

		default:
			throw std::out_of_range("invalid time unit");
	}

	lastUnit = index;
}
