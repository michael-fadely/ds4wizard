#pragma once

#include <QJsonObject>

struct JsonData
{
	virtual ~JsonData() = default;
	virtual void read(const QJsonObject& json) = 0;
	virtual void write(QJsonObject& json) = 0;

	template <typename T>
	static T read(const QJsonObject& json)
	{
		T result {};
		result.read(json);
		return result;
	}
};
