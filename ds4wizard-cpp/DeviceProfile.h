#pragma once

#include <string>
#include <unordered_map>
#include <deque>

#include "DeviceSettingsCommon.h"
#include "Ds4TouchRegion.h"
#include "InputMap.h"

/**
 * \brief A class which represents a collection of input-to-output simulation bindings.
 */
class DeviceProfile : public DeviceSettingsCommon
{
public:
	/**
	 * \brief Gets a filesystem-friendly filename representation of \c name.
	 * \sa name
	 */
	[[nodiscard]] std::string fileName() const;

	/**
	 * \brief User-defined name for the profile.
	 */
	std::string name;

	/**
	 * \brief If \c true, opens an exclusive handle to the device, (mostly) hiding it from other applications.
	 */
	bool exclusiveMode = true;

	/**
	 * \brief If \c true, enables XInput simulation.
	 */
	bool useXInput = true;

	/**
	 * \brief If \c true, automatically selects an XInput player index.
	 */
	bool autoXInputIndex = true;

	/**
	 * \brief User-defined XInput player index.
	 */
	int xinputIndex = 0;

	/**
	 * \brief Collection of touchpad regions referenced by name.
	 */
	std::unordered_map<std::string, Ds4TouchRegion> touchRegions;

	/**
	 * \brief Input-to-output bindings managed by this profile.
	 * \sa InputMap
	 */
	std::deque<InputMap> bindings;

	/**
	 * \brief Modifier sets managed by this profile.
	 * \sa InputModifier
	 */
	std::deque<InputModifier> modifiers;

	DeviceProfile() = default;
	DeviceProfile(const DeviceProfile&) = default;
	DeviceProfile& operator=(const DeviceProfile&) = default;

	bool operator==(const DeviceProfile& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;

	/**
	 * \brief The default representation of a \c DeviceProfile.
	 */
	static DeviceProfile defaultProfile();
};
