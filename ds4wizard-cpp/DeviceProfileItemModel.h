#pragma once

#include <QAbstractListModel>
#include "DeviceProfileCache.h"

class DeviceProfileItemModel final : public QAbstractListModel
{
	Q_OBJECT

	DeviceProfileCache& profileCache;
	EventToken onProfileAdded_;
	EventToken onProfileChanged_;
	EventToken onProfileRemoved_;

	bool includeDefault;

public:
	DeviceProfileItemModel(QObject* parent, DeviceProfileCache& profileCache, bool includeDefault_);

	int rowCount(const QModelIndex& parent) const override;

	int columnCount(const QModelIndex& parent) const override;

	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	DeviceProfile getProfile(int index) const;

private:
	void onProfileAdded(DeviceProfileCache* sender, const DeviceProfile& profile, int index);
	void onProfileChanged(DeviceProfileCache* sender, const DeviceProfile& oldProfile, const DeviceProfile& newProfile, int oldIndex, int newIndex);
	void onProfileRemoved(DeviceProfileCache* sender, const DeviceProfile& profile, int index);
};
