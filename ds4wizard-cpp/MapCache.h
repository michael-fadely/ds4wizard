#pragma once

#include <unordered_map>
#include <unordered_set>

#include "enums.h"
#include "Ds4TouchRegion.h"
#include <set>

template <typename T, typename Map>
class MapCache
{
	using MapSet = std::unordered_set<Map*>;

	std::unordered_set<Map const*> visitedMaps;
	std::unordered_map<T, MapSet> maps;

public:
	MapCache() = default;

	/**
	 * \brief Move constructor.
	 * \param other The instance to move.
	 */
	MapCache(MapCache<T, Map>&& other) noexcept
		: visitedMaps(std::move(other.visitedMaps)),
		  maps(std::move(other.maps))
	{
	}

	/**
	 * \brief Move assignment operator.
	 * \param other The instance to move.
	 * \return Reference to the destination instance.
	 */
	MapCache<T, Map>& operator=(MapCache<T, Map>&& other) noexcept
	{
		visitedMaps = std::move(other.visitedMaps);
		maps        = std::move(other.maps);

		return *this;
	}

	/**
	 * \brief Copy constructor. Disabled.
	 * \param other Instance to copy.
	 */
	MapCache(const MapCache<T, Map>& other) = delete;

	/**
	 * \brief Copy assignment operator. Disabled.
	 * \param other The instance to copy.
	 * \return Reference to the destination instance.
	 */
	MapCache<T, Map>& operator=(const MapCache<T, Map>& other) = delete;

	/**
	 * \brief Clear the visited state of all maps cached by this instance.
	 */
	void reset()
	{
		visitedMaps.clear();
	}

	/**
	 * \brief Clears all the cached maps and their visited state from this instance.
	 */
	void clear()
	{
		reset();
		maps.clear();
	}

	/**
	 * \brief Cache a given \c Map.
	 * \param key They key to associate with the value.
	 * \param value The value to cache.
	 * \return \c true if the value was not already cached.
	 */
	bool cache(const T& key, Map* value)
	{
		if (value == nullptr)
		{
			return false;
		}

		auto it = maps.find(key);

		if (it == maps.end())
		{
			maps[key] = { value };
			return true;
		}

		auto& set = it->second;

		return set.insert(value).second;
	}

	/**
	 * \brief Get a collection of cached \c Map for a given key.
	 * \param key The key to search by.
	 * \return \c std::nullopt if not found, otherwise \c MapSet.
	 */
	std::optional<MapSet> get(const T& key)
	{
		auto it = maps.find(key);

		if (it == maps.end())
		{
			return std::nullopt;
		}

		return maps;
	}

	/**
	 * \brief Marks a given \c Map as visited.
	 * \param value The value to mark as visited.
	 * \return \c false if \p has already been visited or is \c nullptr, else \c true.
	 */
	bool visit(Map const* value)
	{
		if (value == nullptr)
		{
			return false;
		}

		return visitedMaps.insert(value).second;
	}

	/**
	 * \brief Check if a given \c Map has been visited.
	 * \param value The \c Map to check the visited state of.
	 * \return \c true if the \c Map has been visited.
	 */
	[[nodiscard]] bool visited(Map const* value) const
	{
		return visitedMaps.find(value) != visitedMaps.end();
	}
};

/**
 * \brief A collection of input mappings cached by type (button, axis, touch region).
 * \tparam Map The map type to cache.
 * \sa InputType
 */
template <typename Map>
class MapCacheCollection
{
	MapCache<Ds4Buttons_t, Map> buttonMaps;
	MapCache<Ds4Axes_t,    Map> axisMaps;
	MapCache<std::string,  Map> touchMaps;

public:
	/**
	 * \brief Set of all maps managed by this collection.
	 */
	std::set<Map*> set;

	MapCacheCollection() = default;

	/**
	 * \brief Move constructor.
	 * \param other The instance to move.
	 */
	MapCacheCollection(MapCacheCollection<Map>&& other) noexcept
		: buttonMaps(std::move(other.buttonMaps)),
		  axisMaps(std::move(other.axisMaps)),
		  touchMaps(std::move(other.touchMaps))
	{
	}

	/**
	 * \brief Move assignment operator.
	 * \param other The instance to move.
	 * \return Reference to the destination instance.
	 */
	MapCacheCollection<Map>& operator=(MapCacheCollection<Map>&& other) noexcept
	{
		buttonMaps = std::move(other.buttonMaps);
		axisMaps   = std::move(other.axisMaps);
		touchMaps  = std::move(other.touchMaps);

		return *this;
	}

	/**
	 * \brief Copy constructor. Disabled.
	 * \param other The instance to copy.
	 */
	MapCacheCollection(const MapCacheCollection<Map>& other) = delete;

	/**
	 * \brief Copy assignment operator. Disabled.
	 * \param other The instance to copy.
	 * \return Reference to the destination instance.
	 */
	MapCacheCollection<Map>& operator=(const MapCacheCollection<Map>& other) = delete;

	/**
	 * \brief Reset the visited state of all the cached maps managed by this collection.
	 */
	void reset()
	{
		buttonMaps.reset();
		axisMaps.reset();
		touchMaps.reset();
	}

	/**
	 * \brief Clear all the cached maps managed by this collection.
	 */
	void clear()
	{
		set.clear();
		buttonMaps.clear();
		axisMaps.clear();
		touchMaps.clear();
	}

	/**
	 * \brief Cache a collection of \c Map.
	 * \param maps The range of \c Map to be cached.
	 * \param touchRegions Touch region cache to be used for binding validation.
	 */
	void cache(gsl::span<Map> maps, const Ds4TouchRegionCache& touchRegions)
	{
		for (auto& map : maps)
		{
			if (map.inputType & InputType::button && map.inputButtons.has_value())
			{
				for (const Ds4Buttons_t bit : Ds4Buttons_values)
				{
					if (map.inputButtons.value() & bit)
					{
						buttonMaps.cache(bit, &map);
						set.insert(&map);
					}
				}
			}

			if (map.inputType & InputType::axis && map.inputAxes.has_value())
			{
				for (const Ds4Axes_t bit : Ds4Axes_values)
				{
					if (map.inputAxes.value() & bit)
					{
						axisMaps.cache(bit, &map);
						set.insert(&map);
					}
				}
			}

			if (map.inputType & InputType::touchRegion && map.inputTouchRegion.length())
			{
				if (touchRegions.find(map.inputTouchRegion) != touchRegions.cend())
				{
					touchMaps.cache(map.inputTouchRegion, &map);
					set.insert(&map);
				}
			}
		}
	}

	/**
	 * \brief Marks a given \c Map as visited.
	 * \param value The value to mark as visited.
	 * \return \c false if \p has already been visited or is \c nullptr, else \c true.
	 */
	bool visit(Map const* value)
	{
		if (value == nullptr)
		{
			return false;
		}

		bool result = false;

		if (value->inputType & InputType::button && value->inputButtons.has_value())
		{
			result = buttonMaps.visit(value);
		}

		if (value->inputType & InputType::axis && value->inputAxes.has_value())
		{
			result = result || axisMaps.visit(value);
		}

		if (value->inputType & InputType::touchRegion && value->inputTouchRegion.length())
		{
			result = result || touchMaps.visit(value);
		}

		return result;
	}

	/**
	 * \brief Check if a given \c Map has been visited.
	 * \param value The \c Map to check the visited state of.
	 * \return \c true if the \c Map has been visited.
	 */
	[[nodiscard]] bool visited(Map const* value) const
	{
		return buttonMaps.visited(value) ||
		       axisMaps.visited(value) ||
		       touchMaps.visited(value);
	}

	auto getButtonMaps(Ds4Buttons_t key)
	{
		return buttonMaps.get(key);
	}

	auto getAxisMaps(Ds4Axes_t key)
	{
		return axisMaps.get(key);
	}

	auto getTouchMaps(const std::string& key)
	{
		return touchMaps.get(key);
	}
};
