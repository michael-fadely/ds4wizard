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

#if 1
using VirtualKeyCode = int;
#else
enum VirtualKeyCode //: UInt16
{
	/// <summary>
	/// Control-break processing
	/// </summary>
	cancel = 0x03,

	// 0x07 : Undefined

	/// <summary>
	/// BACKSPACE key
	/// </summary>
	backspace = 0x08,

	/// <summary>
	/// TAB key
	/// </summary>
	tab = 0x09,

	// 0x0A - 0x0B : Reserved

	/// <summary>
	/// CLEAR key
	/// </summary>
	clear = 0x0C,

	/// <summary>
	/// ENTER key
	/// </summary>
	enter = 0x0D,

	// 0x0E - 0x0F : Undefined

	/// <summary>
	/// SHIFT key
	/// </summary>
	shift = 0x10,

	/// <summary>
	/// CTRL key
	/// </summary>
	control = 0x11,

	/// <summary>
	/// ALT key
	/// </summary>
	alt = 0x12,

	/// <summary>
	/// PAUSE key
	/// </summary>
	pause = 0x13,

	/// <summary>
	/// CAPS LOCK key
	/// </summary>
	capsLock = 0x14,

	/// <summary>
	/// Input Method Editor (IME) Kana mode
	/// </summary>
	kana = 0x15,

	/// <summary>
	/// IME Hanguel mode (maintained for compatibility; use HANGUL)
	/// </summary>
	hangeul = kana,

	/// <summary>
	/// IME Hangul mode
	/// </summary>
	hangul = kana,

	// 0x16 : Undefined

	/// <summary>
	/// IME Junja mode
	/// </summary>
	junja = 0x17,

	/// <summary>
	/// IME final mode
	/// </summary>
	final = 0x18,

	/// <summary>
	/// IME Hanja mode
	/// </summary>
	hanja = 0x19,

	/// <summary>
	/// IME Kanji mode
	/// </summary>
	kanji = 0x19,

	// 0x1A : Undefined

	/// <summary>
	/// ESC key
	/// </summary>
	escape = 0x1B,

	/// <summary>
	/// IME convert
	/// </summary>
	convert = 0x1C,

	/// <summary>
	/// IME nonconvert
	/// </summary>
	nonConvert = 0x1D,

	/// <summary>
	/// IME accept
	/// </summary>
	accept = 0x1E,

	/// <summary>
	/// IME mode change request
	/// </summary>
	modeChange = 0x1F,

	/// <summary>
	/// SPACEBAR
	/// </summary>
	space = 0x20,

	/// <summary>
	/// PAGE UP key
	/// </summary>
	pageUp = 0x21,

	/// <summary>
	/// PAGE DOWN key
	/// </summary>
	pageDown = 0x22,

	/// <summary>
	/// END key
	/// </summary>
	end = 0x23,

	/// <summary>
	/// HOME key
	/// </summary>
	home = 0x24,

	/// <summary>
	/// LEFT ARROW key
	/// </summary>
	left = 0x25,

	/// <summary>
	/// UP ARROW key
	/// </summary>
	up = 0x26,

	/// <summary>
	/// RIGHT ARROW key
	/// </summary>
	right = 0x27,

	/// <summary>
	/// DOWN ARROW key
	/// </summary>
	down = 0x28,

	/// <summary>
	/// SELECT key
	/// </summary>
	select = 0x29,

	/// <summary>
	/// PRINT key
	/// </summary>
	print = 0x2A,

	/// <summary>
	/// EXECUTE key
	/// </summary>
	execute = 0x2B,

	/// <summary>
	/// PRINT SCREEN key
	/// </summary>
	printScreen = 0x2C,

	/// <summary>
	/// INS key
	/// </summary>
	insert = 0x2D,

	/// <summary>
	/// DEL key
	/// </summary>
	delete_ = 0x2E,

	/// <summary>
	/// HELP key
	/// </summary>
	help = 0x2F,

	/// <summary>
	/// 0 key
	/// </summary>
	digit_0 = 0x30,

	/// <summary>
	/// 1 key
	/// </summary>
	digit_1 = 0x31,

	/// <summary>
	/// 2 key
	/// </summary>
	digit_2 = 0x32,

	/// <summary>
	/// 3 key
	/// </summary>
	digit_3 = 0x33,

	/// <summary>
	/// 4 key
	/// </summary>
	digit_4 = 0x34,

	/// <summary>
	/// 5 key
	/// </summary>
	digit_5 = 0x35,

	/// <summary>
	/// 6 key
	/// </summary>
	digit_6 = 0x36,

	/// <summary>
	/// 7 key
	/// </summary>
	digit_7 = 0x37,

	/// <summary>
	/// 8 key
	/// </summary>
	digit_8 = 0x38,

	/// <summary>
	/// 9 key
	/// </summary>
	digit_9 = 0x39,

	//
	// 0x3A - 0x40 : Undefined
	//

	/// <summary>
	/// A key
	/// </summary>
	a = 0x41,

	/// <summary>
	/// B key
	/// </summary>
	b = 0x42,

	/// <summary>
	/// C key
	/// </summary>
	c = 0x43,

	/// <summary>
	/// D key
	/// </summary>
	d = 0x44,

	/// <summary>
	/// E key
	/// </summary>
	e = 0x45,

	/// <summary>
	/// F key
	/// </summary>
	f = 0x46,

	/// <summary>
	/// G key
	/// </summary>
	g = 0x47,

	/// <summary>
	/// H key
	/// </summary>
	h = 0x48,

	/// <summary>
	/// I key
	/// </summary>
	i = 0x49,

	/// <summary>
	/// J key
	/// </summary>
	j = 0x4A,

	/// <summary>
	/// K key
	/// </summary>
	k = 0x4B,

	/// <summary>
	/// L key
	/// </summary>
	l = 0x4C,

	/// <summary>
	/// M key
	/// </summary>
	m = 0x4D,

	/// <summary>
	/// N key
	/// </summary>
	n = 0x4E,

	/// <summary>
	/// O key
	/// </summary>
	o = 0x4F,

	/// <summary>
	/// P key
	/// </summary>
	p = 0x50,

	/// <summary>
	/// Q key
	/// </summary>
	q = 0x51,

	/// <summary>
	/// R key
	/// </summary>
	r = 0x52,

	/// <summary>
	/// S key
	/// </summary>
	s = 0x53,

	/// <summary>
	/// T key
	/// </summary>
	t = 0x54,

	/// <summary>
	/// U key
	/// </summary>
	u = 0x55,

	/// <summary>
	/// V key
	/// </summary>
	v = 0x56,

	/// <summary>
	/// W key
	/// </summary>
	w = 0x57,

	/// <summary>
	/// X key
	/// </summary>
	x = 0x58,

	/// <summary>
	/// Y key
	/// </summary>
	y = 0x59,

	/// <summary>
	/// Z key
	/// </summary>
	z = 0x5A,

	/// <summary>
	/// Left Windows key (Microsoft Natural keyboard)
	/// </summary>
	windowsKeyLeft = 0x5B,

	/// <summary>
	/// Right Windows key (Natural keyboard)
	/// </summary>
	windowsKeyRight = 0x5C,

	/// <summary>
	/// Applications key (Natural keyboard)
	/// </summary>
	applications = 0x5D,

	// 0x5E : reserved

	/// <summary>
	/// Computer Sleep key
	/// </summary>
	sleep = 0x5F,

	/// <summary>
	/// Numeric keypad 0 key
	/// </summary>
	numpad0 = 0x60,

	/// <summary>
	/// Numeric keypad 1 key
	/// </summary>
	numpad1 = 0x61,

	/// <summary>
	/// Numeric keypad 2 key
	/// </summary>
	numpad2 = 0x62,

	/// <summary>
	/// Numeric keypad 3 key
	/// </summary>
	numpad3 = 0x63,

	/// <summary>
	/// Numeric keypad 4 key
	/// </summary>
	numpad4 = 0x64,

	/// <summary>
	/// Numeric keypad 5 key
	/// </summary>
	numpad5 = 0x65,

	/// <summary>
	/// Numeric keypad 6 key
	/// </summary>
	numpad6 = 0x66,

	/// <summary>
	/// Numeric keypad 7 key
	/// </summary>
	numpad7 = 0x67,

	/// <summary>
	/// Numeric keypad 8 key
	/// </summary>
	numpad8 = 0x68,

	/// <summary>
	/// Numeric keypad 9 key
	/// </summary>
	numpad9 = 0x69,

	/// <summary>
	/// Multiply key
	/// </summary>
	multiply = 0x6A,

	/// <summary>
	/// Add key
	/// </summary>
	add = 0x6B,

	/// <summary>
	/// Separator key
	/// </summary>
	separator = 0x6C,

	/// <summary>
	/// Subtract key
	/// </summary>
	subtract = 0x6D,

	/// <summary>
	/// Decimal key
	/// </summary>
	decimal = 0x6E,

	/// <summary>
	/// Divide key
	/// </summary>
	divide = 0x6F,

	/// <summary>
	/// F1 key
	/// </summary>
	F1 = 0x70,

	/// <summary>
	/// F2 key
	/// </summary>
	F2 = 0x71,

	/// <summary>
	/// F3 key
	/// </summary>
	F3 = 0x72,

	/// <summary>
	/// F4 key
	/// </summary>
	F4 = 0x73,

	/// <summary>
	/// F5 key
	/// </summary>
	F5 = 0x74,

	/// <summary>
	/// F6 key
	/// </summary>
	F6 = 0x75,

	/// <summary>
	/// F7 key
	/// </summary>
	F7 = 0x76,

	/// <summary>
	/// F8 key
	/// </summary>
	F8 = 0x77,

	/// <summary>
	/// F9 key
	/// </summary>
	F9 = 0x78,

	/// <summary>
	/// F10 key
	/// </summary>
	F10 = 0x79,

	/// <summary>
	/// F11 key
	/// </summary>
	F11 = 0x7A,

	/// <summary>
	/// F12 key
	/// </summary>
	F12 = 0x7B,

	/// <summary>
	/// F13 key
	/// </summary>
	F13 = 0x7C,

	/// <summary>
	/// F14 key
	/// </summary>
	F14 = 0x7D,

	/// <summary>
	/// F15 key
	/// </summary>
	F15 = 0x7E,

	/// <summary>
	/// F16 key
	/// </summary>
	F16 = 0x7F,

	/// <summary>
	/// F17 key
	/// </summary>
	F17 = 0x80,

	/// <summary>
	/// F18 key
	/// </summary>
	F18 = 0x81,

	/// <summary>
	/// F19 key
	/// </summary>
	F19 = 0x82,

	/// <summary>
	/// F20 key
	/// </summary>
	F20 = 0x83,

	/// <summary>
	/// F21 key
	/// </summary>
	F21 = 0x84,

	/// <summary>
	/// F22 key
	/// </summary>
	F22 = 0x85,

	/// <summary>
	/// F23 key
	/// </summary>
	F23 = 0x86,

	/// <summary>
	/// F24 key
	/// </summary>
	F24 = 0x87,

	//
	// 0x88 - 0x8F : Unassigned
	//

	/// <summary>
	/// NUM LOCK key
	/// </summary>
	numLock = 0x90,

	/// <summary>
	/// SCROLL LOCK key
	/// </summary>
	scrollLock = 0x91,

	// 0x92 - 0x96 : OEM Specific

	// 0x97 - 0x9F : Unassigned

	//
	// L* & R* - left and right Alt, Ctrl and Shift virtual keys.
	// Used only as parameters to GetAsyncKeyState() and GetKeyState().
	// No other API or message will distinguish left and right keys in this way.
	//

	/// <summary>
	/// Left SHIFT key - Used only as parameters to GetAsyncKeyState() and GetKeyState()
	/// </summary>
	shiftLeft = 0xA0,

	/// <summary>
	/// Right SHIFT key - Used only as parameters to GetAsyncKeyState() and GetKeyState()
	/// </summary>
	shiftRight = 0xA1,

	/// <summary>
	/// Left CONTROL key - Used only as parameters to GetAsyncKeyState() and GetKeyState()
	/// </summary>
	controlLeft = 0xA2,

	/// <summary>
	/// Right CONTROL key - Used only as parameters to GetAsyncKeyState() and GetKeyState()
	/// </summary>
	controlRight = 0xA3,

	/// <summary>
	/// Left MENU key - Used only as parameters to GetAsyncKeyState() and GetKeyState()
	/// </summary>
	altLeft = 0xA4,

	/// <summary>
	/// Right MENU key - Used only as parameters to GetAsyncKeyState() and GetKeyState()
	/// </summary>
	altRight = 0xA5,

	/// <summary>
	/// Windows 2000/XP: Browser Back key
	/// </summary>
	browserBack = 0xA6,

	/// <summary>
	/// Windows 2000/XP: Browser Forward key
	/// </summary>
	browserForward = 0xA7,

	/// <summary>
	/// Windows 2000/XP: Browser Refresh key
	/// </summary>
	browserRefresh = 0xA8,

	/// <summary>
	/// Windows 2000/XP: Browser Stop key
	/// </summary>
	browserStop = 0xA9,

	/// <summary>
	/// Windows 2000/XP: Browser Search key
	/// </summary>
	browserSearch = 0xAA,

	/// <summary>
	/// Windows 2000/XP: Browser Favorites key
	/// </summary>
	browserFavorites = 0xAB,

	/// <summary>
	/// Windows 2000/XP: Browser Start and Home key
	/// </summary>
	browserHome = 0xAC,

	/// <summary>
	/// Windows 2000/XP: Volume Mute key
	/// </summary>
	volumeMute = 0xAD,

	/// <summary>
	/// Windows 2000/XP: Volume Down key
	/// </summary>
	volumeDown = 0xAE,

	/// <summary>
	/// Windows 2000/XP: Volume Up key
	/// </summary>
	volumeUp = 0xAF,

	/// <summary>
	/// Windows 2000/XP: Next Track key
	/// </summary>
	mediaNextTrack = 0xB0,

	/// <summary>
	/// Windows 2000/XP: Previous Track key
	/// </summary>
	mediaPreviousTrack = 0xB1,

	/// <summary>
	/// Windows 2000/XP: Stop Media key
	/// </summary>
	mediaStop = 0xB2,

	/// <summary>
	/// Windows 2000/XP: Play/Pause Media key
	/// </summary>
	mediaPlayPause = 0xB3,

	/// <summary>
	/// Windows 2000/XP: Start Mail key
	/// </summary>
	launchMail = 0xB4,

	/// <summary>
	/// Windows 2000/XP: Select Media key
	/// </summary>
	launchMediaSelect = 0xB5,

	/// <summary>
	/// Windows 2000/XP: Start Application 1 key
	/// </summary>
	launchApplication1 = 0xB6,

	/// <summary>
	/// Windows 2000/XP: Start Application 2 key
	/// </summary>
	launchApplication2 = 0xB7,

	//
	// 0xB8 - 0xB9 : Reserved
	//

	/// <summary>
	/// Used for miscellaneous characters; it can vary by keyboard.
	/// Windows 2000/XP: For the US standard keyboard, the ';:' key 
	/// </summary>
	oem1 = 0xBA,

	/// <summary>
	/// Windows 2000/XP: For any country/region, the '+' key
	/// </summary>
	oemPlus = 0xBB,

	/// <summary>
	/// Windows 2000/XP: For any country/region, the ',' key
	/// </summary>
	oemComma = 0xBC,

	/// <summary>
	/// Windows 2000/XP: For any country/region, the '-' key
	/// </summary>
	oemMinus = 0xBD,

	/// <summary>
	/// Windows 2000/XP: For any country/region, the '.' key
	/// </summary>
	oemPeriod = 0xBE,

	/// <summary>
	/// Used for miscellaneous characters; it can vary by keyboard.
	/// Windows 2000/XP: For the US standard keyboard, the '/?' key 
	/// </summary>
	oem2 = 0xBF,

	/// <summary>
	/// Used for miscellaneous characters; it can vary by keyboard.
	/// Windows 2000/XP: For the US standard keyboard, the '`~' key 
	/// </summary>
	oem3 = 0xC0,

	//
	// 0xC1 - 0xD7 : Reserved
	//

	//
	// 0xD8 - 0xDA : Unassigned
	//

	/// <summary>
	/// Used for miscellaneous characters; it can vary by keyboard.
	/// Windows 2000/XP: For the US standard keyboard, the '[{' key
	/// </summary>
	oem4 = 0xDB,

	/// <summary>
	/// Used for miscellaneous characters; it can vary by keyboard.
	/// Windows 2000/XP: For the US standard keyboard, the '\|' key
	/// </summary>
	oem5 = 0xDC,

	/// <summary>
	/// Used for miscellaneous characters; it can vary by keyboard.
	/// Windows 2000/XP: For the US standard keyboard, the ']}' key
	/// </summary>
	oem6 = 0xDD,

	/// <summary>
	/// Used for miscellaneous characters; it can vary by keyboard.
	/// Windows 2000/XP: For the US standard keyboard, the 'single-quote/double-quote' key
	/// </summary>
	oem7 = 0xDE,

	/// <summary>
	/// Used for miscellaneous characters; it can vary by keyboard.
	/// </summary>
	oem8 = 0xDF,

	//
	// 0xE0 : Reserved
	//

	//
	// 0xE1 : OEM Specific
	//

	/// <summary>
	/// Windows 2000/XP: Either the angle bracket key or the backslash key on the RT 102-key keyboard
	/// </summary>
	oem102 = 0xE2,

	//
	// (0xE3-E4) : OEM specific
	//

	/// <summary>
	/// Windows 95/98/Me, Windows NT 4.0, Windows 2000/XP: IME PROCESS key
	/// </summary>
	processKey = 0xE5,

	//
	// 0xE6 : OEM specific
	//

	/// <summary>
	/// Windows 2000/XP: Used to pass Unicode characters as if they were keystrokes.
	/// The PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods.
	/// For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP
	/// </summary>
	packet = 0xE7,

	//
	// 0xE8 : Unassigned
	//

	//
	// 0xE9-F5 : OEM specific
	//

	/// <summary>
	/// Attn key
	/// </summary>
	attn = 0xF6,

	/// <summary>
	/// CrSel key
	/// </summary>
	crSel = 0xF7,

	/// <summary>
	/// ExSel key
	/// </summary>
	exSel = 0xF8,

	/// <summary>
	/// Erase EOF key
	/// </summary>
	erEOF = 0xF9,

	/// <summary>
	/// Play key
	/// </summary>
	play = 0xFA,

	/// <summary>
	/// Zoom key
	/// </summary>
	zoom = 0xFB,

	/// <summary>
	/// Reserved
	/// </summary>
	noName = 0xFC,

	/// <summary>
	/// PA1 key
	/// </summary>
	pa1 = 0xFD,

	/// <summary>
	/// Clear key
	/// </summary>
	oemClear = 0xFE,
};
#endif

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
