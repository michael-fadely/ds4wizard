#pragma once
#include "Pressable.h"
#include "Stopwatch.h"
#include "AxisOptions.h"

class InputMapBase : public Pressable, public JsonData
{
public:
	/*[JsonIgnore]*/ bool isToggled = false;

	/*[JsonIgnore]*/
	bool performRapidFire() const;

private:
	/*[JsonIgnore]*/ bool rapidFiring = false;
	/*[JsonIgnore]*/ PressedState rapidState = PressedState::off;
	/*[JsonIgnore]*/ Stopwatch rapidStopwatch;

public:
	/*[JsonIgnore]*/
	PressedState simulatedState() const;

	/*[JsonIgnore]*/
	virtual bool isActive() const override;

	/// <summary>
	/// Indicates if this instance has a persistent state.
	/// </summary>
	/*[JsonIgnore]*/
	bool isPersistent() const;

	InputType_t inputType = 0;

	std::optional<Ds4Buttons_t> inputButtons;
	std::optional<Ds4Axis_t> inputAxis;
	std::string  inputRegion;

	std::optional<bool> toggle;
	std::optional<bool> rapidFire;

	std::optional<std::chrono::nanoseconds> rapidFireInterval;

	std::unordered_map<Ds4Axis_t, InputAxisOptions> inputAxisOptions;

	InputMapBase() = default;
	InputMapBase(const InputMapBase& other);

	explicit InputMapBase(InputType_t inputType);
	InputMapBase(InputType_t inputType, Ds4Buttons::T input);
	InputMapBase(InputType_t inputType, Ds4Axis::T input);
	InputMapBase(InputType_t inputType, const std::string& input);

	virtual void press() override;

protected:
	void UpdateRapidState();

public:
	virtual void release() override;
	InputAxisOptions getAxisOptions(Ds4Axis_t axis);

	bool operator==(const InputMapBase& other) const;
	bool operator!=(const InputMapBase& other) const;

	virtual void readJson(const QJsonObject& json) override;
	virtual void writeJson(QJsonObject& json) const override;

	// TODO
#if 0
	public override string ToString()
	{
		var builder = new StringBuilder();
		builder.AppendFormat("[{0}] ", InputType);

		if (InputType == InputType.None)
		{
			builder.Append("N/A");
			return builder.ToString();
		}

		if ((InputType & InputType.button) != 0)
		{
			if (InputButtons != null)
			{
				builder.Append(InputButtons.Value);
			}
		}

		if ((InputType & InputType.axis) != 0 && InputAxis.HasValue)
		{
			foreach (Ds4Axis bit in Enum.GetValues(typeof(Ds4Axis)))
			{
				// ReSharper disable once PossibleInvalidOperationException
				if ((InputAxis.Value & bit) == 0)
				{
					continue;
				}

				InputAxisOptions options = GetAxisOptions(bit);

				if (options.Polarity == AxisPolarity.Negative)
				{
					builder.Append("-");
				}
				else
				{
					builder.Append("+");
				}

				if (InputAxis != null)
				{
					builder.Append(InputAxis.Value);
				}
			}
		}

		if ((InputType & InputType.touchRegion) != 0)
		{
			builder.Append(InputRegion);
		}

		return builder.ToString();
	}
#endif
};

class InputMap;

class InputModifier : public InputMapBase
{
public:
	/// <summary>
	/// The bindings associated with this modifier set.
	/// </summary>
	std::list<InputMap> bindings;

	InputModifier() = default;

	InputModifier(InputType_t type, Ds4Buttons::T buttons);
	InputModifier(InputType_t type, Ds4Axis::T axis);
	InputModifier(InputType_t type, const std::string& region);

	// Copy constructor
	InputModifier(const InputModifier& other);

	bool operator==(const InputModifier& other) const;
	bool operator!=(const InputModifier& other) const;
	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;
};

class InputMap : public InputMapBase
{
public:
	SimulatorType simulatorType = SimulatorType::none;
	OutputType_t  outputType = 0;

	std::optional<ActionType> action;

	#pragma region Touch

	std::optional<Direction_t> touchDirection;

	#pragma endregion

	#pragma region Keyboard

	// TODO: VirtualKeyCode?      KeyCode;
	// TODO: std::list<VirtualKeyCode> KeyCodeModifiers;

	#pragma endregion

	#pragma region Mouse

	MouseAxes mouseAxes;
	// TODO: use internal mouse button bitfield
	// TODO: MouseButton? MouseButton;

	#pragma endregion

	#pragma region XInput

	std::optional<XInputButtons_t> xinputButtons = 0;
	std::optional<XInputAxes     > xinputAxes;

	#pragma endregion

	InputMap() = default;

	/// <summary>
	/// Copy constructor.
	/// </summary>
	/// <param name="other">The instance to copy from.</param>
	InputMap(const InputMap& other);

	InputMap(SimulatorType simulatorType, InputType_t inputType, OutputType::T outputType);

	void pressModifier(const InputModifier* modifier);

	bool operator==(const InputMap& other) const;
	bool operator!=(const InputMap& other) const;

	void readJson(const QJsonObject& json) override;
	void writeJson(QJsonObject& json) const override;

	// TODO
#if 0
	public override string ToString()
	{
		var builder = new StringBuilder();

		builder.Append(base.ToString());
		builder.AppendFormat("; [{0}] ", SimulatorType);

		if (SimulatorType == SimulatorType.Action)
		{
			builder.Append(Action ?? ActionType.None);
		}

		if (OutputType == OutputType.None)
		{
			builder.AppendFormat(" [{0}]", OutputType.None);
			return builder.ToString();
		}

		bool appended = false;

		if ((OutputType & OutputType.XInput) != 0)
		{
			builder.Append("[XInput] ");

			if (XInputButtons != null)
			{
				builder.AppendFormat("Buttons: {0}", XInputButtons.Value);
				appended = true;
			}

			if (XInputAxes != null)
			{
				if (appended)
				{
					builder.Append(", ");
				}

				builder.AppendFormat("axis: {0}", XInputAxes);
				appended = true;
			}
		}

		if (appended)
		{
			builder.Append(" ");
		}

		if ((OutputType & OutputType.Keyboard) != 0)
		{
			builder.Append("[Keyboard] ");
			appended = true;

			bool modifiers = false;
			if (KeyCodeModifiers != null && KeyCodeModifiers.Count > 0)
			{
				modifiers = true;
				builder.AppendFormat("{0}",
					string.Join("+", KeyCodeModifiers.Select(x => x.ToString())));
			}

			if (KeyCode != null)
			{
				if (modifiers)
				{
					builder.Append("+");
				}

				builder.Append(KeyCode.Value);
			}
		}

		if (appended)
		{
			builder.Append(" ");
		}

		if ((OutputType & OutputType.Mouse) != 0)
		{
			builder.Append("[Mouse] ");
			appended = true;

			bool hasButtons = false;
			if (MouseButton != null)
			{
				hasButtons = true;
				builder.AppendFormat("button: {0}", MouseButton.Value);
			}

			if (MouseAxes != null)
			{
				if (hasButtons)
				{
					builder.Append(", ");
				}

				builder.AppendFormat("Mouse Directions: {0}", MouseAxes);
			}
		}

		if (appended)
		{
			builder.Append(" ");
		}

		return builder.ToString();
	}
#endif
};
