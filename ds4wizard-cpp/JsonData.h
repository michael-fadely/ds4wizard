#pragma once

#include <nlohmann/json.hpp>

/**
 * \brief A simple interface for de/serializable JSON objects.
 */
struct JsonData
{
	virtual ~JsonData() = default;
	virtual void readJson(const nlohmann::json& json) = 0;
	virtual void writeJson(nlohmann::json& json) const = 0;

	nlohmann::json toJson() const
	{
		nlohmann::json object;
		writeJson(object);
		return object;
	}

	template <typename T>
	static T fromJson(const nlohmann::json& json)
	{
		T result {};
		result.readJson(json);
		return result;
	}
};
