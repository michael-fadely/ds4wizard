#pragma once

#include <string>
#include <optional>
#include <deque>

#include "Pressable.h"
#include "Stopwatch.h"
#include "AxisOptions.h"

class InputMapBase : public Pressable, public JsonData
{
public:
	bool isToggled = false;
	bool performRapidFire() const;

private:
	bool rapidFiring = false;
	PressedState rapidState = PressedState::off;
	Stopwatch rapidStopwatch;

public:
	~InputMapBase() override = default;

	/**
	 * \brief
	 * The pressed state of the underlying emulated mapping.
	 * For example if \c toggle and \c isToggled are \c true,
	 * this function will return \c PressedState::pressed or \c PressedState::on.
	 * \sa PressedState
	 */
	[[nodiscard]] PressedState simulatedState() const;

	[[nodiscard]] bool isActive() const override;

	/**
	 * \brief Indicates if this instance has a persistent state
	 * which is actively simulated.
	 */
	[[nodiscard]] bool isPersistent() const;

	InputType_t inputType = 0;

	std::optional<Ds4Buttons_t> inputButtons;
	std::optional<Ds4Axes_t> inputAxes;
	std::string inputTouchRegion;
	std::optional<Direction_t> inputTouchDirection;

	std::optional<bool> toggle;
	std::optional<bool> rapidFire;

	std::optional<std::chrono::microseconds> rapidFireInterval;

	std::unordered_map<Ds4Axes_t, InputAxisOptions> inputAxisOptions;

	InputMapBase() = default;
	InputMapBase(const InputMapBase& other);
	InputMapBase(InputMapBase&& other) noexcept;

	explicit InputMapBase(InputType_t inputType);
	InputMapBase(InputType_t inputType, Ds4Buttons::T input);
	InputMapBase(InputType_t inputType, Ds4Axes::T input);
	InputMapBase(InputType_t inputType, std::string input);

	InputMapBase& operator=(const InputMapBase&) = default;
	InputMapBase& operator=(InputMapBase&& other) noexcept;

	void press() override;

protected:
	void updateRapidState();

public:
	void release() override;
	InputAxisOptions getAxisOptions(Ds4Axes_t axis) const;

	bool operator==(const InputMapBase& other) const;
	bool operator!=(const InputMapBase& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};

class InputModifier;

using VirtualKeyCode = int;

class InputMap : public InputMapBase
{
public:
	SimulatorType simulatorType = SimulatorType::none;
	OutputType_t outputType     = 0;

	std::optional<ActionType> action;

	#pragma region Keyboard

	std::optional<VirtualKeyCode> keyCode;
	std::vector<VirtualKeyCode>   keyCodeModifiers;

	#pragma endregion

	#pragma region Mouse

	std::optional<MouseAxes> mouseAxes;
	std::optional<MouseButton> mouseButton;

	#pragma endregion

	#pragma region XInput

	std::optional<XInputButtons_t> xinputButtons;
	std::optional<XInputAxes> xinputAxes;

	#pragma endregion

	InputMap() = default;
	~InputMap() override = default;

	InputMap(const InputMap& other) = default;
	InputMap(InputMap&& other) noexcept;

	InputMap(SimulatorType simulatorType, InputType_t inputType, OutputType::T outputType);

	InputMap& operator=(const InputMap& other) = default;
	InputMap& operator=(InputMap&& other) noexcept;

	/**
	 * \brief Activates a press state on this instance.
	 * If a modifier is provided, it must be active for the press to succeed.
	 * \param modifier The parent modifier, if any.
	 */
	void pressWithModifier(const InputModifier* modifier);

	bool operator==(const InputMap& other) const;
	bool operator!=(const InputMap& other) const;

	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};

/**
 * \brief A modifier set that controls a collection of input bindings.
 */
class InputModifier : public InputMapBase
{
public:
	/**
	* \brief The bindings associated with this modifier set.
	*/
	std::deque<InputMap> bindings;

	InputModifier() = default;
	~InputModifier() override = default;

	InputModifier(InputType_t type, Ds4Buttons::T buttons);
	InputModifier(InputType_t type, Ds4Axes::T axis);
	InputModifier(InputType_t type, const std::string& region);

	// Copy constructor
	InputModifier(const InputModifier& other);

	InputModifier(InputModifier&& other) noexcept;

	InputModifier& operator=(const InputModifier&) = default;
	InputModifier& operator=(InputModifier&& other) noexcept;

	bool operator==(const InputModifier& other) const;
	bool operator!=(const InputModifier& other) const;
	void readJson(const nlohmann::json& json) override;
	void writeJson(nlohmann::json& json) const override;
};
