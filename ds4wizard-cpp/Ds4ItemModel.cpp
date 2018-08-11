#include "stdafx.h"
#include "Ds4ItemModel.h"
#include <QMetaType>

Ds4ItemModel::Ds4ItemModel(std::shared_ptr<Ds4DeviceManager> deviceManager)
	: deviceManager(std::move(deviceManager))
{
	qRegisterMetaType<std::shared_ptr<DeviceOpenedEventArgs>>("std::shared_ptr<DeviceOpenedEventArgs>");
	qRegisterMetaType<std::shared_ptr<DeviceClosedEventArgs>>("std::shared_ptr<DeviceClosedEventArgs>");

	connect(this, &Ds4ItemModel::s_onDeviceOpened, this, &Ds4ItemModel::onDeviceOpened);
	connect(this, &Ds4ItemModel::s_onDeviceClosed, this, &Ds4ItemModel::onDeviceClosed);
	connect(this, &Ds4ItemModel::s_onDeviceBatteryChanged, this, &Ds4ItemModel::onDeviceBatteryChanged);

	this->deviceManager->deviceOpened += [this](void*, std::shared_ptr<DeviceOpenedEventArgs> args) -> void
	{
		emit s_onDeviceOpened(args);
	};

	this->deviceManager->deviceClosed += [this](void*, std::shared_ptr<DeviceClosedEventArgs> args) -> void
	{
		emit s_onDeviceClosed(args);
	};
}

int Ds4ItemModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(deviceManager->deviceCount());
}

int Ds4ItemModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 2; // device name, battery
}

QVariant Ds4ItemModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || index.row() < 0 || static_cast<size_t>(index.row()) >= deviceManager->deviceCount())
	{
		return QVariant();
	}

	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	std::shared_ptr<Ds4Device> device = getDevice(index.row());

	switch (index.column())
	{
		case 0:
			return QString::fromStdString(device->name());

		case 1:
			return QString("%1%").arg(device->battery() * 10);

		default:
			return QVariant();
	}
}

QVariant Ds4ItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (section > 1)
	{
		return QVariant();
	}

	if (orientation != Qt::Horizontal)
	{
		return QVariant();
	}

	switch (static_cast<Qt::ItemDataRole>(role))
	{
		case Qt::DisplayRole:
			return tr(!section ? "Device" : "Battery");

		default:
			return QVariant();
	}
}

void Ds4ItemModel::onDeviceOpened(std::shared_ptr<DeviceOpenedEventArgs> a)
{
	if (a->unique)
	{
		auto device = a->device;

		device->batteryLevelChanged += [this](void* sender, std::shared_ptr<EventArgs>) -> void
		{
			emit s_onDeviceBatteryChanged(reinterpret_cast<Ds4Device*>(sender));
		};
	}

	const auto index = createIndex(getRow(a->device), 0);
	emit dataChanged(index, index, { Qt::ItemDataRole::DisplayRole });
}

void Ds4ItemModel::onDeviceClosed(std::shared_ptr<DeviceClosedEventArgs> a)
{
	const auto index = createIndex(getRow(a->device), 0);
	emit dataChanged(index, index, { Qt::ItemDataRole::DisplayRole });
}

void Ds4ItemModel::onDeviceBatteryChanged(const Ds4Device* sender)
{
	const auto index = createIndex(getRow(sender), 1);
	emit dataChanged(index, index, { Qt::ItemDataRole::DisplayRole });
}

std::shared_ptr<Ds4Device> Ds4ItemModel::getDevice(int row) const
{
	auto lock = deviceManager->lockDevices();
	auto it = deviceManager->devices.cbegin();

	for (int i = 0; i < row && it != deviceManager->devices.end(); ++i)
	{
		++it;
	}

	if (it == deviceManager->devices.end())
	{
		return nullptr;
	}

	return it->second;
}

int Ds4ItemModel::getRow(const std::shared_ptr<Ds4Device>& device) const
{
	return getRow(device.get());
}

int Ds4ItemModel::getRow(const Ds4Device* device) const
{
	auto lock = deviceManager->lockDevices();
	auto it = deviceManager->devices.cbegin();

	int row = -1;

	while (it != deviceManager->devices.cend())
	{
		++row;

		if (it->second.get() == device)
		{
			break;
		}

		++it;
	}

	return row;
}
