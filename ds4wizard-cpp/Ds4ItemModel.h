#pragma once

#include <QAbstractItemModel>

class Ds4ItemModel : public QAbstractItemModel
{
	std::shared_ptr<Ds4DeviceManager> deviceManager;

public:
	Ds4ItemModel(std::shared_ptr<Ds4DeviceManager> device_manager);

	QModelIndex index(int row, int column, const QModelIndex& parent) const override;

	QModelIndex parent(const QModelIndex& child) const override;

	int rowCount(const QModelIndex& parent) const override;

	int columnCount(const QModelIndex& parent) const override;

	QVariant data(const QModelIndex& index, int role) const override;
};
