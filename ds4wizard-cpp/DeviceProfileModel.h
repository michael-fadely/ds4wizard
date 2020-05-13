#pragma once
#include <QAbstractListModel>
#include "DeviceProfile.h"

class ModifierListModel final : public QAbstractListModel
{
	Q_OBJECT

	DeviceProfile* profile = nullptr;
	bool includeNone = false;

public:
	ModifierListModel(QObject* parent, DeviceProfile* profile_);

	int rowCount(const QModelIndex& parent) const override;

	int columnCount(const QModelIndex& parent) const override;

	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

class DeviceProfileModel final : public QObject
{
	Q_OBJECT

	DeviceProfile profile;

public:
	DeviceProfileModel(QObject* parent, DeviceProfile&& profile_);
	DeviceProfileModel(QObject* parent, const DeviceProfile& profile_);
	~DeviceProfileModel() override = default;

	[[nodiscard]] ModifierListModel* makeModifierModel(QObject* parent);
};
