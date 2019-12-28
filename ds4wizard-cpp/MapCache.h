#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "enums.h"
#include "Ds4TouchRegion.h"

template <typename Key, typename Map>
class MapCache
{
public:
	using Key_t = Key;
	using Map_t = Map;

	/**
	 * \brief The underlying set type used to store the cache.
	 */
	using MapSet = std::unordered_set<Map_t*>;

	/**
	 * \brief Type declaration for the callback used by \sa visit
	 */
	using Visitor = std::function<bool(Map_t*)>;

private:
	MapSet allMaps_ {};
	MapSet visitedMaps {};
	std::unordered_map<Key_t, MapSet> maps {};

public:
	/**
	 * \brief A set of all maps managed by this instance.
	 */
	[[nodiscard]] MapSet& allMaps() const
	{
		return allMaps_;
	}

	MapCache() = default;

	/**
	 * \brief Move constructor.
	 * \param other The instance to move.
	 */
	MapCache(MapCache<Key_t, Map_t>&& other) noexcept
		: visitedMaps(std::move(other.visitedMaps)),
		  maps(std::move(other.maps))
	{
	}

	/**
	 * \brief Move assignment operator.
	 * \param other The instance to move.
	 * \return Reference to the destination instance.
	 */
	MapCache<Key_t, Map_t>& operator=(MapCache<Key_t, Map_t>&& other) noexcept
	{
		visitedMaps = std::move(other.visitedMaps);
		maps        = std::move(other.maps);

		return *this;
	}

	/**
	 * \brief Copy constructor. Disabled.
	 * \param other Instance to copy.
	 */
	MapCache(const MapCache<Key_t, Map_t>& other) = delete;

	/**
	 * \brief Copy assignment operator. Disabled.
	 * \param other The instance to copy.
	 * \return Reference to the destination instance.
	 */
	MapCache<Key_t, Map_t>& operator=(const MapCache<Key_t, Map_t>& other) = delete;

	/**
	 * \brief Clear the visited state of all maps cached by this instance.
	 */
	void reset()
	{
		visitedMaps.clear();
	}

	/**
	 * \brief Clears all the cached maps from this instance.
	 */
	void clear()
	{
		reset();
		allMaps_.clear();
		maps.clear();
	}

	/**
	 * \brief Cache a given \c Map_t.
	 * \param key They key to associate with the value.
	 * \param value The value to cache.
	 * \return \c true if the value was not already cached.
	 */
	bool cache(const Key_t& key, Map_t* value)
	{
		if (value == nullptr)
		{
			return false;
		}

		allMaps_.insert(value);

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
	 * \brief Get a collection of cached \c Map_t for a given key.
	 * \param key The key to search by.
	 * \return A pointer to the \c MapSet found for the given key, or \c nullptr.
	 */
	MapSet* get(const Key_t& key)
	{
		auto it = maps.find(key);

		if (it == maps.end())
		{
			return nullptr;
		}

		return &it->second;
	}

	/**
	 * \brief Marks a given \c Map_t as visited.
	 * \param value The value to mark as visited.
	 * \return \c false if \p has already been visited or is \c nullptr, else \c true.
	 */
	bool markVisited(Map_t* value)
	{
		if (value == nullptr)
		{
			return false;
		}

		if (allMaps_.find(value) == allMaps_.cend())
		{
			return false;
		}

		return visitedMaps.insert(value).second;
	}

	void visit(const Key_t& key, Visitor visitor)
	{
		MapSet* collection = get(key);

		if (collection == nullptr)
		{
			return;
		}

		for (Map_t* map : *collection)
		{
			if (wasVisited(map))
			{
				continue;
			}

			if (visitor(map))
			{
				markVisited(map);
			}
		}
	}

	/**
	 * \brief Check if a given \c Map_t has been visited.
	 * \param value The \c Map_t to check the visited state of.
	 * \return \c true if the \c Map_t has been visited.
	 */
	[[nodiscard]] bool wasVisited(Map_t* value) const
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
	std::unordered_set<Map*> allMaps_;

public:
	/**
	 * \brief Set of all maps managed by this collection.
	 */
	[[nodiscard]] const auto& allMaps() const
	{
		return allMaps_;
	}

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
		allMaps_.clear();
		buttonMaps.clear();
		axisMaps.clear();
		touchMaps.clear();
	}

	/** TODO: use range for maps
	 * \brief Cache a collection of \c Map.
	 * \param maps The range of \c Map to be cached.
	 * \param touchRegions Touch region cache to be used for binding validation.
	 */
	void cache(std::deque<Map> maps, const Ds4TouchRegionCache& touchRegions)
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
						allMaps_.insert(&map);
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
						allMaps_.insert(&map);
					}
				}
			}

			if (map.inputType & InputType::touchRegion && map.inputTouchRegion.length())
			{
				if (touchRegions.find(map.inputTouchRegion) != touchRegions.cend())
				{
					touchMaps.cache(map.inputTouchRegion, &map);
					allMaps_.insert(&map);
				}
			}
		}
	}

	/**
	 * \brief Marks a given \c Map as visited.
	 * \param value The value to mark as visited.
	 * \return \c false if \p has already been visited or is \c nullptr, else \c true.
	 */
	bool markVisited(Map* value)
	{
		if (value == nullptr)
		{
			return false;
		}

		bool result = false;

		if (value->inputType & InputType::button && value->inputButtons.has_value())
		{
			result = buttonMaps.markVisited(value);
		}

		if (value->inputType & InputType::axis && value->inputAxes.has_value())
		{
			result = result || axisMaps.markVisited(value);
		}

		if (value->inputType & InputType::touchRegion && value->inputTouchRegion.length())
		{
			result = result || touchMaps.markVisited(value);
		}

		return result;
	}

	/**
	 * \brief Check if a given \c Map has been visited.
	 * \param value The \c Map to check the visited state of.
	 * \return \c true if the \c Map has been visited.
	 */
	[[nodiscard]] bool wasVisited(Map* value) const
	{
		return buttonMaps.wasVisited(value) ||
		       axisMaps.wasVisited(value) ||
		       touchMaps.wasVisited(value);
	}

	typename decltype(buttonMaps)::MapSet* getButtonMaps(Ds4Buttons_t key)
	{
		return buttonMaps.get(key);
	}

	typename decltype(axisMaps)::MapSet* getAxisMaps(Ds4Axes_t key)
	{
		return axisMaps.get(key);
	}

	typename decltype(touchMaps)::MapSet* getTouchMaps(const std::string& key)
	{
		return touchMaps.get(key);
	}

	void visitButtonMaps(typename decltype(buttonMaps)::Key_t key, typename decltype(buttonMaps)::Visitor visitor)
	{
		buttonMaps.visit(key, visitor);
	}

	void visitAxisMaps(typename decltype(axisMaps)::Key_t key, typename decltype(axisMaps)::Visitor visitor)
	{
		axisMaps.visit(key, visitor);
	}

	void visitTouchMaps(typename decltype(touchMaps)::Key_t key, typename decltype(touchMaps)::Visitor visitor)
	{
		touchMaps.visit(key, visitor);
	}
};
