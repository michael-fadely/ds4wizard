#pragma once

#include <string>
#include <unordered_map>
#include <list>

#include "DeviceSettingsCommon.h"
#include "Ds4TouchRegion.h"

// TODO
struct InputMap
{
	int hi;
	bool operator==(const InputMap& o) const { return false; }
	bool operator!=(const InputMap& o) const { return false; }
};

// TODO
struct InputModifier
{
	int hi;
	bool operator==(const InputModifier& o) const { return false; }
	bool operator!=(const InputModifier& o) const { return false; }
};

class DeviceProfile : public DeviceSettingsCommon
{
private:
	/*[JsonIgnore]*/ std::string name_;

public:
	/*[JsonIgnore]*/ std::string FileName;

	std::string Name() const;
	void Name(const std::string& value);

	bool ExclusiveMode;
	bool UseXInput;
	bool AutoXInputIndex;
	int  XInputIndex;

	std::unordered_map<std::string, Ds4TouchRegion> TouchRegions;

	std::list<InputMap>      Bindings;
	std::list<InputModifier> Modifiers;

	DeviceProfile() = default;

	DeviceProfile(const DeviceProfile& other);

	bool operator==(const DeviceProfile& other) const;

	// TODO
	/*static readonly DeviceProfile Default = new DeviceProfile
	{
		Name            = std::string.Empty,
		ExclusiveMode   = true,
		UseXInput       = true,
		AutoXInputIndex = true,
		XInputIndex     = 0,

		Light = new Ds4LightOptions(new Ds4Color(Color.Blue)),
		Idle  = new DeviceIdleOptions(DeviceIdleOptions.Default),

		TouchRegions = new Dictionary<std::string, Ds4TouchRegion>
		{
			{ "Left Half", new Ds4TouchRegion(Ds4TouchRegionType.Button, 0, 0, 959, 941) },
			{
				"Right Half",
				new Ds4TouchRegion(Ds4TouchRegionType.StickAutoCenter, 960, 0, 1919, 941)
				{
					TouchAxisOptions = new Dictionary<Direction, InputAxisOptions>
					{
						{ Direction.Up,    new InputAxisOptions { DeadZone = 0.25f } },
						{ Direction.Down,  new InputAxisOptions { DeadZone = 0.25f } },
						{ Direction.Left,  new InputAxisOptions { DeadZone = 0.25f } },
						{ Direction.Right, new InputAxisOptions { DeadZone = 0.25f } }
					}
				}
			}
		},

		Modifiers = new List<InputModifier>
		{
			new InputModifier(InputType.TouchRegion, "Left Half")
			{
				Bindings = new List<InputMap>
				{
					new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
					{
						InputButtons      = Ds4Buttons.Cross,
						RapidFire         = true,
						RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0),
						XInputButtons     = XInputButtons.A
					},
					new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
					{
						InputButtons      = Ds4Buttons.Circle,
						RapidFire         = true,
						RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0),
						XInputButtons     = XInputButtons.B
					},
					new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
					{
						InputButtons      = Ds4Buttons.Square,
						RapidFire         = true,
						RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0),
						XInputButtons     = XInputButtons.X
					},
					new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
					{
						InputButtons      = Ds4Buttons.Triangle,
						RapidFire         = true,
						RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0),
						XInputButtons     = XInputButtons.Y
					},
					new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
					{
						InputButtons      = Ds4Buttons.Options,
						RapidFire         = true,
						RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0),
						XInputButtons     = XInputButtons.Start
					},
					new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
					{
						InputButtons      = Ds4Buttons.Share,
						RapidFire         = true,
						RapidFireInterval = TimeSpan.FromMilliseconds(1000.0 / 60.0),
						XInputButtons     = XInputButtons.Back
					}
				}
			},
			new InputModifier(InputType.TouchRegion, "Right Half")
			{
				Bindings = new List<InputMap>
				{
					new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
					{
						InputButtons  = Ds4Buttons.TouchButton,
						XInputButtons = XInputButtons.LeftThumb
					}
				}
			},
			new InputModifier(InputType.Button, Ds4Buttons.PS)
			{
				Bindings = new List<InputMap>
				{
					new InputMap(SimulatorType.Action, InputType.Button, OutputType.None)
					{
						InputButtons = Ds4Buttons.Options,
						Action       = ActionType.BluetoothDisconnect
					}
				}
			}
		},

		Bindings = new List<InputMap>
		{
			#region Touch Binds

			new InputMap(SimulatorType.Input, InputType.TouchRegion, OutputType.XInput)
			{
				InputRegion    = "Right Half",
				TouchDirection = Direction.Up,
				XInputButtons  = XInputButtons.DPadUp
			},
			new InputMap(SimulatorType.Input, InputType.TouchRegion, OutputType.XInput)
			{
				InputRegion    = "Right Half",
				TouchDirection = Direction.Down,
				XInputButtons  = XInputButtons.DPadDown
			},
			new InputMap(SimulatorType.Input, InputType.TouchRegion, OutputType.XInput)
			{
				InputRegion    = "Right Half",
				TouchDirection = Direction.Left,
				XInputButtons  = XInputButtons.DPadLeft
			},
			new InputMap(SimulatorType.Input, InputType.TouchRegion, OutputType.XInput)
			{
				InputRegion    = "Right Half",
				TouchDirection = Direction.Right,
				XInputButtons  = XInputButtons.DPadRight
			},

			#endregion

			#region Buttons

			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.Cross,
				XInputButtons = XInputButtons.A
			},
			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.Circle,
				XInputButtons = XInputButtons.B
			},
			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.Square,
				XInputButtons = XInputButtons.X
			},
			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.Triangle,
				XInputButtons = XInputButtons.Y
			},

			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.L1,
				XInputButtons = XInputButtons.LeftShoulder
			},
			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.R1,
				XInputButtons = XInputButtons.RightShoulder
			},
			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.L3,
				XInputButtons = XInputButtons.LeftThumb
			},
			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.R3,
				XInputButtons = XInputButtons.RightThumb
			},

			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.Options,
				XInputButtons = XInputButtons.Start
			},
			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.Share,
				XInputButtons = XInputButtons.Back
			},

			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.Up,
				XInputButtons = XInputButtons.DPadUp
			},
			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.Down,
				XInputButtons = XInputButtons.DPadDown
			},
			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.Left,
				XInputButtons = XInputButtons.DPadLeft
			},
			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.Right,
				XInputButtons = XInputButtons.DPadRight
			},

			new InputMap(SimulatorType.Input, InputType.Button, OutputType.XInput)
			{
				InputButtons  = Ds4Buttons.PS,
				XInputButtons = XInputButtons.Guide
			},

			#endregion

			#region Axes

			new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
			{
				InputAxis = Ds4Axis.LeftStickY,

				InputAxisOptions = new Dictionary<Ds4Axis, InputAxisOptions>
				{
					{ Ds4Axis.LeftStickY, new InputAxisOptions(AxisPolarity.Positive) }
				},

				XInputAxes = new XInputAxes
				{
					Axes = XInputAxis.LeftStickY,
					Options = new Dictionary<XInputAxis, AxisOptions>
					{
						{ XInputAxis.LeftStickY, new AxisOptions(AxisPolarity.Positive) }
					}
				}
			},
			new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
			{
				InputAxis = Ds4Axis.LeftStickY,

				InputAxisOptions = new Dictionary<Ds4Axis, InputAxisOptions>
				{
					{ Ds4Axis.LeftStickY, new InputAxisOptions(AxisPolarity.Negative) }
				},

				XInputAxes = new XInputAxes
				{
					Axes = XInputAxis.LeftStickY,
					Options = new Dictionary<XInputAxis, AxisOptions>
					{
						{ XInputAxis.LeftStickY, new AxisOptions(AxisPolarity.Negative) }
					}
				}
			},
			new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
			{
				InputAxis = Ds4Axis.LeftStickX,

				InputAxisOptions = new Dictionary<Ds4Axis, InputAxisOptions>
				{
					{ Ds4Axis.LeftStickX, new InputAxisOptions(AxisPolarity.Positive) }
				},

				XInputAxes = new XInputAxes
				{
					Axes = XInputAxis.LeftStickX,
					Options = new Dictionary<XInputAxis, AxisOptions>
					{
						{ XInputAxis.LeftStickX, new AxisOptions(AxisPolarity.Positive) }
					}
				}
			},
			new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
			{
				InputAxis = Ds4Axis.LeftStickX,

				InputAxisOptions = new Dictionary<Ds4Axis, InputAxisOptions>
				{
					{ Ds4Axis.LeftStickX, new InputAxisOptions(AxisPolarity.Negative) }
				},

				XInputAxes = new XInputAxes
				{
					Axes = XInputAxis.LeftStickX,
					Options = new Dictionary<XInputAxis, AxisOptions>
					{
						{ XInputAxis.LeftStickX, new AxisOptions(AxisPolarity.Negative) }
					}
				}
			},

			new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
			{
				InputAxis = Ds4Axis.RightStickY,

				InputAxisOptions = new Dictionary<Ds4Axis, InputAxisOptions>
				{
					{ Ds4Axis.RightStickY, new InputAxisOptions(AxisPolarity.Positive) }
				},

				XInputAxes = new XInputAxes
				{
					Axes = XInputAxis.RightStickY,
					Options = new Dictionary<XInputAxis, AxisOptions>
					{
						{ XInputAxis.RightStickY, new AxisOptions(AxisPolarity.Positive) }
					}
				}
			},
			new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
			{
				InputAxis = Ds4Axis.RightStickY,

				InputAxisOptions = new Dictionary<Ds4Axis, InputAxisOptions>
				{
					{ Ds4Axis.RightStickY, new InputAxisOptions(AxisPolarity.Negative) }
				},

				XInputAxes = new XInputAxes
				{
					Axes = XInputAxis.RightStickY,
					Options = new Dictionary<XInputAxis, AxisOptions>
					{
						{ XInputAxis.RightStickY, new AxisOptions(AxisPolarity.Negative) }
					}
				}
			},
			new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
			{
				InputAxis = Ds4Axis.RightStickX,

				InputAxisOptions = new Dictionary<Ds4Axis, InputAxisOptions>
				{
					{ Ds4Axis.RightStickX, new InputAxisOptions(AxisPolarity.Positive) }
				},

				XInputAxes = new XInputAxes
				{
					Axes = XInputAxis.RightStickX,
					Options = new Dictionary<XInputAxis, AxisOptions>
					{
						{ XInputAxis.RightStickX, new AxisOptions(AxisPolarity.Positive) }
					}
				}
			},
			new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
			{
				InputAxis = Ds4Axis.RightStickX,

				InputAxisOptions = new Dictionary<Ds4Axis, InputAxisOptions>
				{
					{ Ds4Axis.RightStickX, new InputAxisOptions(AxisPolarity.Negative) }
				},

				XInputAxes = new XInputAxes
				{
					Axes = XInputAxis.RightStickX,
					Options = new Dictionary<XInputAxis, AxisOptions>
					{
						{ XInputAxis.RightStickX, new AxisOptions(AxisPolarity.Negative) }
					}
				}
			},

			new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
			{
				InputAxis = Ds4Axis.LeftTrigger,
				XInputAxes = new XInputAxes
				{
					Axes = XInputAxis.LeftTrigger
				}
			},
			new InputMap(SimulatorType.Input, InputType.Axis, OutputType.XInput)
			{
				InputAxis = Ds4Axis.RightTrigger,
				XInputAxes = new XInputAxes
				{
					Axes = XInputAxis.RightTrigger
				}
			}

			#endregion
		}
	};*/
};
