#include "pch.h"
#include <iomanip>

#include <shellapi.h>

#include <fmt/format.h>
#include <fmt/xchar.h>

#include <hid_util.h>
#include <devicetoggle.h>

#include <QTranslator>

#include "Ds4Device.h"
#include "lock.h"
#include "Logger.h"
#include "program.h"
#include "stringutil.h"

#include "Ds4DeviceManager.h"

/*
 * TODO: this needs to be re-done; see reasons below
 * - a cooldown time should be specified on toggles for a device.
 */

Ds4DeviceManager::~Ds4DeviceManager()
{
	close();
}

bool Ds4DeviceManager::isDs4(const hid::HidInstance& hid)
{
	return hid.attributes().vendorId == vendorId &&
	       std::ranges::find(productIds, hid.attributes().productId) != productIds.end();
}

bool Ds4DeviceManager::isDs4(const std::wstring& devicePath)
{
	hid::HidInstance hid(devicePath);
	return hid.readMetadata() && isDs4(hid);
}

void Ds4DeviceManager::findControllers()
{
	MAKE_GUARD(sync_lock);

	hid::enumerateHid([&](std::shared_ptr<hid::HidInstance> hid) -> bool
	{
		return handleDevice(hid);
	});
}

void Ds4DeviceManager::findController(const std::wstring& devicePath)
{
	if (!isDs4(devicePath))
	{
		return;
	}

	GUID guid = {};
	HidD_GetHidGuid(&guid);

	MAKE_GUARD(sync_lock);

	hid::enumerateGuid([&](const std::wstring& path, const std::wstring& instanceId) -> bool
	{
		if (!iequals(path, devicePath))
		{
			return false;
		}

		auto hid = std::make_shared<hid::HidInstance>(path, instanceId);

		if (hid->readMetadata())
		{
			return handleDevice(hid);
		}

		return false;
	}, guid);
}

size_t Ds4DeviceManager::deviceCount()
{
	LOCK(devices);
	return devices.size();
}

std::unique_lock<std::recursive_mutex> Ds4DeviceManager::lockDevices()
{
	return std::unique_lock(devices_lock);
}

void Ds4DeviceManager::registerDeviceCallbacks(const std::wstring& serialString, const std::shared_ptr<Ds4Device>& device)
{
	auto& token_store = tokens.insert({ serialString, {} }).first->second;

	token_store.push_back(device->onDeviceClose.add([this](Ds4Device* sender) { onDs4DeviceClose(sender); }));

	auto onConnect = [](Ds4Device* sender, const Ds4ConnectEvent& args)
	{
		switch (args.connectionType)
		{
			case ConnectionType::usb:
			{
				switch (args.status)
				{
					case Ds4ConnectEvent::Status::opened:
						Logger::writeLine(LogLevel::info, sender->name(), QObject::tr("USB connected.").toStdString());
						break;

					case Ds4ConnectEvent::Status::toggleFailed:
						Logger::writeLine(LogLevel::error, sender->name(), QObject::tr("USB toggle failed. Please report!").toStdString());
						break;

					case Ds4ConnectEvent::Status::exclusiveFailed:
						Logger::writeLine(LogLevel::warning, sender->name(), QObject::tr("USB connected, but exclusive acquisition failed.").toStdString());
						break;

					case Ds4ConnectEvent::Status::openFailed:
						Logger::writeLine(LogLevel::error, sender->name(), QObject::tr("USB open failed!").toStdString());
						break;

					default:
						break;
				}
				break;
			}

			case ConnectionType::bluetooth:
			{
				switch (args.status)
				{
					case Ds4ConnectEvent::Status::opened:
						Logger::writeLine(LogLevel::info, sender->name(), QObject::tr("Bluetooth connected.").toStdString());
						break;

					case Ds4ConnectEvent::Status::toggleFailed:
						Logger::writeLine(LogLevel::error, sender->name(), QObject::tr("Bluetooth toggle failed. Please report!").toStdString());
						break;

					case Ds4ConnectEvent::Status::exclusiveFailed:
						Logger::writeLine(LogLevel::warning, sender->name(), QObject::tr("Bluetooth connected, but exclusive acquisition failed.").toStdString());
						break;

					case Ds4ConnectEvent::Status::openFailed:
						Logger::writeLine(LogLevel::error, sender->name(), QObject::tr("Bluetooth open failed!").toStdString());
						break;

					default:
						break;
				}
				break;
			}

			default:
				break;
		}
	};

	auto onDisconnect = [](Ds4Device* sender, const Ds4DisconnectEvent& args)
	{
		switch (args.connectionType)
		{
			case ConnectionType::usb:
			{
				switch (args.reason)
				{
					case Ds4DisconnectEvent::Reason::dropped:
						Logger::writeLine(LogLevel::error, sender->name(), QObject::tr("USB connection lost!").toStdString());
						break;

					case Ds4DisconnectEvent::Reason::closed:
						Logger::writeLine(LogLevel::info, sender->name(), QObject::tr("USB disconnected.").toStdString());
						break;

					case Ds4DisconnectEvent::Reason::idle:
						Logger::writeLine(LogLevel::info, sender->name(), QObject::tr("Controller idle. USB disconnected.").toStdString());
						break;

					case Ds4DisconnectEvent::Reason::error:
					{
						const QString str = QObject::tr("USB disconnected unexpectedly with error code %1").arg(args.nativeError.value_or(0));
						Logger::writeLine(LogLevel::error, sender->name(), str.toStdString());
						break;
					}

					default:
						break;
				}
				break;
			}

			case ConnectionType::bluetooth:
			{
				switch (args.reason)
				{
					case Ds4DisconnectEvent::Reason::dropped:
						Logger::writeLine(LogLevel::error, sender->name(), QObject::tr("Bluetooth connection lost!").toStdString());
						break;

					case Ds4DisconnectEvent::Reason::closed:
						Logger::writeLine(LogLevel::info, sender->name(), QObject::tr("Bluetooth disconnected.").toStdString());
						break;

					case Ds4DisconnectEvent::Reason::idle:
						Logger::writeLine(LogLevel::info, sender->name(), QObject::tr("Controller idle. Bluetooth disconnected.").toStdString());
						break;

					case Ds4DisconnectEvent::Reason::error:
					{
						const QString str = QObject::tr("Bluetooth disconnected unexpectedly with error code %1").arg(args.nativeError.value_or(0));
						Logger::writeLine(LogLevel::error, sender->name(), str.toStdString());
						break;
					}

					default:
						break;
				}
				break;
			}

			default:
				break;
		}
	};

	token_store.push_back(device->onConnect.add(onConnect));
	token_store.push_back(device->onConnectFailure.add(onConnect));
	token_store.push_back(device->onDisconnect.add(onDisconnect));

	token_store.push_back(device->onWirelessOperationalModeFailure.add(
		[](Ds4Device* sender, size_t nativeError) -> void
		{
			const QString str = QObject::tr("Enabling wireless operational mode failed with error code %1").arg(nativeError);
			Logger::writeLine(LogLevel::error, sender->name(), str.toStdString());
		}));

	token_store.push_back(device->onLatencyThresholdExceeded.add(
		[](Ds4Device* sender, std::chrono::milliseconds value, std::chrono::milliseconds threshold)
		{
			const QString str = QObject::tr("Input latency has exceeded the threshold. (%1 ms > %2 ms)")
			                    .arg(value.count())
			                    .arg(threshold.count());
			
			Logger::writeLine(LogLevel::warning, sender->name(), str.toStdString());
		}));

	token_store.push_back(device->onBatteryFullyCharged.add([](Ds4Device* sender)
		{
			Logger::writeLine(LogLevel::info, sender->name(), QObject::tr("Fully charged.").toStdString());
		}));

	token_store.push_back(device->onBatteryLevelLow.add([](Ds4Device* sender, uint8_t value)
		{
			const QString str = QObject::tr("Battery running low! (%1%)").arg(value * 10);
			Logger::writeLine(LogLevel::warning, sender->name(), str.toStdString());
		}));
}

bool Ds4DeviceManager::handleDevice(const std::shared_ptr<hid::HidInstance>& hid)
{
	if (!isDs4(*hid))
	{
		return false;
	}

	MAKE_GUARD(sync_lock);

	bool isBluetooth;

	try
	{
		isBluetooth = hid->caps().inputReportSize != Ds4Device::usbInputReportSize;

		// USB connection type
		if (!isBluetooth)
		{
			MacAddress macAddress = Ds4Device::getMacAddress(hid);

			// HACK: this member shouldn't be exposed, but getting the "serial" (MAC) over USB is non-standard for the DS4.
			hid->serialString = fmt::format(L"{:2x}{:2x}{:2x}{:2x}{:2x}{:2x}",
			                                macAddress[0], macAddress[1], macAddress[2],
			                                macAddress[3], macAddress[4], macAddress[5]);
		}

		if (hid->serialString.empty())
		{
			const std::string hidPath(hid->path.begin(), hid->path.end());
			throw std::runtime_error(fmt::format("Device {0} returned empty MAC address.", hidPath));
		}
	}
	catch (const std::exception& ex)
	{
		// TODO: proper HID exceptions
		Logger::writeLine(LogLevel::warning, "Failed to read device metadata: " + std::string(ex.what()));
		hid->close();
		return false;
	}

	try
	{
		LOCK(devices);

		const auto it = devices.find(hid->serialString);

		// device isn't already being managed, so set up all the event handling/etc
		if (it == devices.end())
		{
			const auto& serialString = hid->serialString;

			std::shared_ptr<Ds4Device> device = std::make_shared<Ds4Device>();

			registerDeviceCallbacks(serialString, device);

			device->open(hid);

			const auto& safe = device->safeMacAddress();

			const std::wstring wstr(safe.cbegin(), safe.cend());
			devices[wstr] = device;

			auto args = std::make_shared<DeviceOpenedEventArgs>(device, true);
			deviceOpened.invoke(this, args);
			device->start();
		}
		else
		{
			std::shared_ptr<Ds4Device>& device = it->second;

			if (isBluetooth)
			{
				if (!device->bluetoothConnected())
				{
					device->openBluetoothDevice(hid);
				}
			}
			else
			{
				if (!device->usbConnected())
				{
					device->openUsbDevice(hid);
				}
			}

			auto args = std::make_shared<DeviceOpenedEventArgs>(device, false);
			deviceOpened.invoke(this, args);
		}
	}
	catch (const std::exception& ex)
	{
		// TODO: proper HID exceptions
		Logger::writeLine(LogLevel::error, "Error while opening device: " + std::string(ex.what()));

		LOCK(devices);
		devices.erase(hid->serialString);
		tokens.erase(hid->serialString);
	}

	return false;
}

void Ds4DeviceManager::onDs4DeviceClose(Ds4Device* sender)
{
	LOCK(devices);

	const auto mac = std::wstring(sender->safeMacAddress().cbegin(), sender->safeMacAddress().cend());
	const auto it = devices.find(mac);

	if (it == devices.end())
	{
		return;
	}

	std::shared_ptr<Ds4Device> ptr = it->second;

	auto args = std::make_shared<DeviceClosedEventArgs>(ptr);
	deviceClosed.invoke(this, args);
	tokens.erase(mac);
	devices.erase(it);
}

void Ds4DeviceManager::close()
{
	decltype(devices) devices_;

	{
		LOCK(devices);
		devices_ = std::move(devices);
	}

	for (auto& it : devices_)
	{
		const std::shared_ptr<Ds4Device> ptr = std::move(it.second);
		ptr->close();

		auto args = std::make_shared<DeviceClosedEventArgs>(ptr);
		deviceClosed.invoke(this, args);

		const auto mac = std::wstring(ptr->safeMacAddress().cbegin(), ptr->safeMacAddress().cend());
		tokens.erase(mac);
	}
}

void Ds4DeviceManager::toggleDevice(const std::wstring& instanceId)
{
	// TODO: use CreateProcess and get console output

	if (Program::isElevated())
	{
		::toggleDevice(instanceId);
		return;
	}

	const std::wstring params = fmt::format(L"--toggle-device \"{0}\"", instanceId);

	SHELLEXECUTEINFO info {};
	info.cbSize       = sizeof(info);
	info.fMask        = SEE_MASK_NOCLOSEPROCESS;
	info.lpFile       = L"ds4wizard-device-toggle.exe";
	info.lpParameters = params.c_str();
	info.lpVerb       = L"runas";

	if (!ShellExecuteExW(&info))
	{
		throw std::runtime_error("ShellExecuteExW failed");
	}

	Handle handle(info.hProcess, true);

	WaitForSingleObject(handle.nativeHandle, INFINITE);

	DWORD exitCode = 0;
	if (!GetExitCodeProcess(handle.nativeHandle, &exitCode) || exitCode != 0)
	{
		handle.close();
		throw std::runtime_error("Device toggle failed! Please report.");
	}
}
