#pragma once

#define QT_IS_FUCKING_BROKEN

// Windows
#include <Windows.h>
#include <SetupAPI.h>

#include <winioctl.h>
#include <BluetoothAPIs.h>
#include <Bthioctl.h>

#include <Dbt.h>
#include <Hidsdi.h>
#include <shellapi.h>
#include <Xinput.h>

// STL
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <deque>
#include <functional>
#include <iomanip>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

// Qt
#include <QDialog>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets>

// GSL
#include <gsl/span>

// better-enums
#include <enum.h>

// fmt
#include <fmt/format.h>

// libhid
#include <hid_handle.h>
#include <hid_instance.h>
#include <hid_util.h>

#include "average.h"
#include "AxisOptions.h"
#include "Bluetooth.h"
#include "busenum.h"
#include "ConnectionType.h"
#include "DeviceIdleOptions.h"
#include "DeviceProfile.h"
#include "DeviceProfileCache.h"
#include "DevicePropertiesDialog.h"
#include "DeviceSettings.h"
#include "DeviceSettingsCommon.h"
#include "Ds4AutoLightColor.h"
#include "Ds4Color.h"
#include "Ds4Device.h"
#include "Ds4DeviceManager.h"
#include "Ds4Input.h"
#include "Ds4InputData.h"
#include "Ds4ItemModel.h"
#include "Ds4LightOptions.h"
#include "Ds4Output.h"
#include "Ds4TouchRegion.h"
#include "enums.h"
#include "Event.h"
#include "InputMap.h"
#include "InputSimulator.h"
#include "JsonData.h"
#include "KeyboardSimulator.h"
#include "Latency.h"
#include "lerp.h"
#include "lock.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MouseSimulator.h"
#include "pathutil.h"
#include "Pressable.h"
#include "ProfileEditorDialog.h"
#include "program.h"
#include "ScpDevice.h"
#include "Settings.h"
#include "Stopwatch.h"
#include "stringutil.h"
#include "XInputGamepad.h"