#pragma once

#include <QJsonObject>

struct JsonData
{
	virtual ~JsonData() = default;
	virtual void readJson(const QJsonObject& json) = 0;
	virtual void writeJson(QJsonObject& json) const = 0;

	QJsonObject toJson() const
	{
		QJsonObject object;
		writeJson(object);
		return object;
	}

	template <typename T>
	static T fromJson(const QJsonObject& json)
	{
		T result {};
		result.readJson(json);
		return result;
	}
};
