#include "stdafx.h"
#include "Ds4ItemModel.h"
#include <QMetaType>

Ds4ItemModel::Ds4ItemModel(const std::shared_ptr<Ds4DeviceManager>& deviceManager)
{
	qRegisterMetaType<std::shared_ptr<DeviceOpenedEventArgs>>("std::shared_ptr<DeviceOpenedEventArgs>");
	qRegisterMetaType<std::shared_ptr<DeviceClosedEventArgs>>("std::shared_ptr<DeviceClosedEventArgs>");

	connect(this, &Ds4ItemModel::s_onDeviceOpened, this, &Ds4ItemModel::onDeviceOpened);
	connect(this, &Ds4ItemModel::s_onDeviceClosed, this, &Ds4ItemModel::onDeviceClosed);
	connect(this, &Ds4ItemModel::s_onDeviceBatteryChanged, this, &Ds4ItemModel::onDeviceBatteryChanged);

	deviceManager->deviceOpened += [this](void*, std::shared_ptr<DeviceOpenedEventArgs> args) -> void
	{
		emit s_onDeviceOpened(args);
	};

	deviceManager->deviceClosed += [this](void*, std::shared_ptr<DeviceClosedEventArgs> args) -> void
	{
		emit s_onDeviceClosed(args);
	};
}

int Ds4ItemModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(devices.size());
}

int Ds4ItemModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 2; // device name, battery
}

QVariant Ds4ItemModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || index.row() < 0 || static_cast<size_t>(index.row()) >= devices.size())
	{
		return QVariant();
	}

	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	const std::shared_ptr<Ds4Device> device = getDevice(index.row());

	switch (index.column())
	{
		case 0:
			return QString::fromStdString(device->name());

		case 1:
			if (device->charging())
			{
				return QString("+%1%").arg(device->battery() * 10);
			}
			else
			{
				return QString("%1%").arg(device->battery() * 10);
			}

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
	devices.insert(a->device);
	const auto row = getRow(a->device);

	if (a->unique)
	{
		auto device = a->device;

		device->batteryLevelChanged += [this](void* sender, std::shared_ptr<EventArgs>) -> void
		{
			emit s_onDeviceBatteryChanged(reinterpret_cast<Ds4Device*>(sender));
		};

		beginInsertRows({}, row, row);
		endInsertRows();
	}
	else
	{
		const auto index = createIndex(row, 0);
		const auto index2 = createIndex(row, columnCount({}) - 1);
		emit dataChanged(index, index2);
	}
}

void Ds4ItemModel::onDeviceClosed(std::shared_ptr<DeviceClosedEventArgs> a)
{
	const auto row = getRow(a->device);

	beginRemoveRows({}, row, row);
	devices.erase(a->device);
	endRemoveRows();
}

void Ds4ItemModel::onDeviceBatteryChanged(const Ds4Device* sender)
{
	const auto index = createIndex(getRow(sender), 1);
	emit dataChanged(index, index);
}

std::shared_ptr<Ds4Device> Ds4ItemModel::getDevice(int row) const
{
	auto it = devices.cbegin();

	for (int i = 0; i < row && it != devices.cend(); ++i)
	{
		++it;
	}

	if (it == devices.cend())
	{
		return nullptr;
	}

	return *it;
}

int Ds4ItemModel::getRow(const std::shared_ptr<Ds4Device>& device) const
{
	return getRow(device.get());
}

int Ds4ItemModel::getRow(const Ds4Device* device) const
{
	auto it = devices.cbegin();

	int row = -1;

	while (it != devices.cend())
	{
		++row;

		if ((*it).get() == device)
		{
			break;
		}

		++it;
	}

	return row;
}
