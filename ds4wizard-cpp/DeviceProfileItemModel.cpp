#include "pch.h"
#include "DeviceProfileCache.h"
#include "DeviceProfileItemModel.h"

DeviceProfileItemModel::DeviceProfileItemModel(QObject* parent, DeviceProfileCache& profileCache, bool includeDefault_)
	: QAbstractListModel(parent), profileCache(profileCache), includeDefault(includeDefault_)
{
	this->onProfileAdded_ = profileCache.profileAdded.add(
	[this](DeviceProfileCache* sender, const DeviceProfile& profile, int index)
	{
		onProfileAdded(sender, profile, index);
	});

	this->onProfileChanged_ = profileCache.profileChanged.add(
	[this](DeviceProfileCache* sender, const DeviceProfile& oldProfile, const DeviceProfile& newProfile, int oldIndex, int newIndex)
	{
		onProfileChanged(sender, oldProfile, newProfile, oldIndex, newIndex);
	});

	this->onProfileRemoved_ = profileCache.profileRemoved.add(
	[this](DeviceProfileCache* sender, const DeviceProfile& profile, int index)
	{
		onProfileRemoved(sender, profile, index);
	});
}

int DeviceProfileItemModel::rowCount(const QModelIndex& parent) const
{
	return static_cast<int>(profileCache.profiles.size()) + static_cast<int>(includeDefault);
}

int DeviceProfileItemModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

QVariant DeviceProfileItemModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || index.row() < 0 || role != Qt::DisplayRole)
	{
		return QVariant();
	}

	if (includeDefault)
	{
		if (static_cast<size_t>(index.row()) > profileCache.profiles.size())
		{
			return QVariant();
		}
	}
	else if (static_cast<size_t>(index.row()) >= profileCache.profiles.size())
	{
		return QVariant();
	}

	if (includeDefault && !index.row())
	{
		return tr("[Default]");
	}

	const auto row = index.row() - static_cast<int>(includeDefault);

	return QString::fromStdString(profileCache.profiles[row].name);
}

QVariant DeviceProfileItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (section > 0)
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
			return tr("Profile");

		default:
			return QVariant();
	}
}

void DeviceProfileItemModel::onProfileAdded(DeviceProfileCache* sender, const DeviceProfile& profile, int index)
{
	index += static_cast<int>(includeDefault);
	beginInsertRows({}, index, index);
	endInsertRows();
}

void DeviceProfileItemModel::onProfileChanged(DeviceProfileCache* sender, const DeviceProfile& oldProfile, const DeviceProfile& newProfile, int oldIndex, int newIndex)
{
	oldIndex += static_cast<int>(includeDefault);
	newIndex += static_cast<int>(includeDefault);

	if (oldIndex >= 0)
	{
		beginRemoveRows({}, oldIndex, oldIndex);
		endRemoveRows();
	}

	beginInsertRows({}, newIndex, newIndex);
	endRemoveRows();
}

void DeviceProfileItemModel::onProfileRemoved(DeviceProfileCache* sender, const DeviceProfile& profile, int index)
{
	index += static_cast<int>(includeDefault);
	beginRemoveRows({}, index, index);
	endRemoveRows();
}
