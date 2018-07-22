#include "stdafx.h"
#include "Ds4ItemModel.h"

Ds4ItemModel::Ds4ItemModel(std::shared_ptr<Ds4DeviceManager> deviceManager)
	: deviceManager(std::move(deviceManager))
{
}

QModelIndex Ds4ItemModel::index(int row, int column, const QModelIndex& parent) const
{
	return QModelIndex();
}

QModelIndex Ds4ItemModel::parent(const QModelIndex& child) const
{
	return QModelIndex();
}

int Ds4ItemModel::rowCount(const QModelIndex& parent) const
{
	return static_cast<int>(deviceManager->deviceCount());
}

int Ds4ItemModel::columnCount(const QModelIndex& parent) const
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

	auto lock = deviceManager->lockDevices();
	auto it = deviceManager->devices.cbegin();

	for (int i = 0; i < index.row() && it != deviceManager->devices.end(); ++i)
	{
		++it;
	}

	QString qstr = QString::fromStdString(it->second->name());
	return qstr;
}
