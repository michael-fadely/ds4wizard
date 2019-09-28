#include "pch.h"
#include "DeviceProfileModel.h"

ModifierListModel::ModifierListModel(QObject* parent, DeviceProfile* profile_)
	: QAbstractListModel(parent),
	  profile(profile_)
{
}

int ModifierListModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(profile->modifiers.size()) + static_cast<int>(includeNone);
}

int ModifierListModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 1;
}

QVariant ModifierListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || index.row() < 0 || role != Qt::DisplayRole)
	{
		return QVariant();
	}

	if (includeNone)
	{
		if (static_cast<size_t>(index.row()) > profile->modifiers.size())
		{
			return QVariant();
		}
	}
	else if (static_cast<size_t>(index.row()) >= profile->modifiers.size())
	{
		return QVariant();
	}

	if (includeNone && !index.row())
	{
		return tr("[None]");
	}

	const auto row = index.row() - static_cast<int>(includeNone);

	return QString::number(row); // TODO: names for modifier sets? use button images?
}

QVariant ModifierListModel::headerData(int section, Qt::Orientation orientation, int role) const
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
			return tr("Modifier");

		default:
			return QVariant();
	}
}

DeviceProfileModel::DeviceProfileModel(QObject* parent, DeviceProfile&& profile_)
	: QObject(parent), profile(std::move(profile_))
{
}

DeviceProfileModel::DeviceProfileModel(QObject* parent, const DeviceProfile& profile_)
	: QObject(parent), profile(profile_)
{
}

ModifierListModel* DeviceProfileModel::makeModifierModel(QObject* parent)
{
	return new ModifierListModel(parent, &profile);
}
