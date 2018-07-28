#pragma once

#include <QAbstractListModel>
#include <tuple>

class Ds4ItemModel : public QAbstractListModel
{
	Q_OBJECT

	std::shared_ptr<Ds4DeviceManager> deviceManager;

public:
	explicit Ds4ItemModel(std::shared_ptr<Ds4DeviceManager> deviceManager);

	int rowCount(const QModelIndex& parent) const override;

	int columnCount(const QModelIndex& parent) const override;

	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
	std::tuple<std::wstring, std::shared_ptr<Ds4Device>> getDevice(int row) const;
	int getRow(const std::shared_ptr<Ds4Device>& device) const;
	int getRow(const Ds4Device* device) const;

signals:
	void s_onDeviceOpened(std::shared_ptr<DeviceOpenedEventArgs> a);
	void s_onDeviceClosed(std::shared_ptr<DeviceClosedEventArgs> a);
	void s_onDeviceBatteryChanged(const Ds4Device* sender);

protected slots:
	void onDeviceOpened(std::shared_ptr<DeviceOpenedEventArgs> a);
	void onDeviceClosed(std::shared_ptr<DeviceClosedEventArgs> a);
	void onDeviceBatteryChanged(const Ds4Device* sender);
};
