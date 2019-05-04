#include "pch.h"
#include "MainWindow.h"
#include "DevicePropertiesDialog.h"
#include "DeviceProfileCache.h"
#include "program.h"
#include "Ds4DeviceManager.h"
#include "Logger.h"
#include "DeviceProfileModel.h"

using namespace std::chrono;

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	const auto now = []() -> auto { return high_resolution_clock::now(); };
	const auto start = now();

	ui.setupUi(this);

	const auto elapsed = now() - start;

	qInfo() << __FUNCTION__ " setupUi: "
		<< duration_cast<milliseconds>(elapsed).count()
		<< " ms";

	ui.checkMinimizeToTray->setChecked(Program::settings.minimizeToTray);
	ui.checkStartMinimized->setChecked(Program::settings.startMinimized);
	ui.comboConnectionType->setCurrentIndex(static_cast<int>(Program::settings.preferredConnection));

	this->supportsSystemTray = QSystemTrayIcon::isSystemTrayAvailable();

	if (supportsSystemTray)
	{
		auto loadIconTask = std::async(std::launch::async, [now]() -> auto
		{
			const auto start = now();
			QIcon icon(":/ds4wizardcpp/Resources/race_q00.ico");
			const auto elapsed = now() - start;
			qInfo() << "icon load time: " << duration_cast<milliseconds>(elapsed).count();
			return icon;
		});

		auto menu = new QMenu(this);

		QAction* action = menu->addAction(tr("Show/Hide"));
		connect(action, &QAction::triggered, this, &MainWindow::systemTrayShowHide);

		menu->addSeparator();

		action = menu->addAction(tr("&Exit"));
		connect(action, &QAction::triggered, this, &MainWindow::systemTrayExit);

		trayIcon = new QSystemTrayIcon(this);
		trayIcon->setContextMenu(menu);

		connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::toggleHide);

		trayIcon->setIcon(loadIconTask.get());
		trayIcon->show();
	}

	this->onLineLogged_ = Logger::lineLogged.add([this](auto a, auto b) -> void { onLineLogged(a, b); });

	deviceManager = std::make_shared<Ds4DeviceManager>();
	Program::profileCache.setDevices(deviceManager);

	connect(this, &MainWindow::s_onProfilesLoaded, this, &MainWindow::onProfilesLoaded);

	ds4Items = new Ds4ItemModel(nullptr, deviceManager);
	ui.deviceList->setModel(ds4Items);

	profileItems = new DeviceProfileItemModel(nullptr, Program::profileCache, false);
	ui.profileList->setModel(profileItems);

	const auto deviceSelectionModel = ui.deviceList->selectionModel();
	connect(deviceSelectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::deviceSelectionChanged);

	// HACK: for testing later
	connect(ui.profileEdit, &QPushButton::clicked, this, [this](...)
	{
		auto model = std::make_unique<DeviceProfileModel>(this, profileItems->getProfile(ui.profileList->currentIndex().row()));
		auto dg = std::make_unique<ProfileEditorDialog>(model.get(), this);
		dg->exec();
	});

	// HACK: for testing later
	/*connect(ui.profileAdd, &QPushButton::clicked, this, [](...)
	{
		Program::profileCache.updateProfile({}, DeviceProfile::defaultProfile());
	});*/

	if (!supportsSystemTray || !Program::settings.startMinimized)
	{
		show();
	}

	// TODO: only start this once the window is shown (or whatever else, since it can start minimized!)
	startupTask = std::async(std::launch::async, [this]() -> void
	{
		Program::profileCache.load();
		emit s_onProfilesLoaded();
		deviceManager->findControllers();
	});
}

MainWindow::~MainWindow()
{
	delete profileItems;
	delete ds4Items;

	deviceManager->close();
	delete trayIcon;
}

void MainWindow::changeEvent(QEvent* e)
{
	switch (e->type())
	{
		case QEvent::WindowStateChange:
			if (!Program::settings.minimizeToTray)
			{
				break;
			}

			if (windowState() & Qt::WindowMinimized)
			{
				if (supportsSystemTray && ui.checkMinimizeToTray->isChecked())
				{
					QTimer::singleShot(0, this, &MainWindow::hide);
				}
			}
			break;

		default:
			break;
	}

	QMainWindow::changeEvent(e);
}

void MainWindow::onLineLogged(void* /*sender*/, std::shared_ptr<LineLoggedEventArgs> args) const
{
	if (!supportsSystemTray)
	{
		return;
	}

	QSystemTrayIcon::MessageIcon icon;
	QString title;

	switch (args->level)
	{
		default:
		case LogLevel::info:
			icon = QSystemTrayIcon::MessageIcon::Information;
			title = tr("Info");
			break;

		case LogLevel::warning:
			icon = QSystemTrayIcon::MessageIcon::Warning;
			title = tr("Warning");
			break;

		case LogLevel::error:
			icon = QSystemTrayIcon::MessageIcon::Critical;
			title = tr("Error");
			break;
	}

	trayIcon->showMessage(title, QString::fromStdString(args->line), icon);
}

void MainWindow::registerDeviceNotification()
{
	GUID guid {};
	HidD_GetHidGuid(&guid);

	DEV_BROADCAST_DEVICEINTERFACE filter {};

	filter.dbcc_size       = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	filter.dbcc_classguid  = guid;
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

	auto hwnd = reinterpret_cast<HWND>(this->winId());

	notificationHandle = RegisterDeviceNotification(hwnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

	if (notificationHandle == nullptr || notificationHandle == INVALID_HANDLE_VALUE)
	{
		QMessageBox::warning(this, tr("Warning"),
		                     tr("Failed to register device notification event. "
		                        "The program will be unable to detect newly connected controllers."));
	}
}

void MainWindow::unregisterDeviceNotification()
{
	if (notificationHandle == nullptr || notificationHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	UnregisterDeviceNotification(notificationHandle);
	notificationHandle = nullptr;
}

void MainWindow::doFindControllerThing(const std::wstring& name) const
{
	try
	{
		deviceManager->findController(name);
	}
	catch (const std::exception&)
	{
		// HACK: ignored
	}
}

bool MainWindow::wndProc(tagMSG* msg) const
{
	if (msg->message == WM_DEVICECHANGE && msg->wParam == DBT_DEVICEARRIVAL)
	{
		auto hdr = reinterpret_cast<DEV_BROADCAST_HDR*>(msg->lParam);

		if (!hdr || hdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
		{
			return false;
		}

		auto devInterface = reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE*>(msg->lParam);
		doFindControllerThing(devInterface->dbcc_name);
	}

	return false;
}

#if !defined(QT_IS_FUCKING_BROKEN)
bool MainWindow::nativeEvent(const QByteArray& /*eventType*/, void* message, long* /*result*/)
{
	auto msg = static_cast<MSG*>(message);
	return wndProc(msg);
}
#endif

void MainWindow::closeEvent(QCloseEvent* /*event*/)
{
	unregisterDeviceNotification();
}

void MainWindow::toggleHide(QSystemTrayIcon::ActivationReason reason)
{
	if (reason != QSystemTrayIcon::DoubleClick)
	{
		return;
	}

	if (isVisible())
	{
		hide();
	}
	else
	{
		setWindowState(Qt::WindowActive);
		show();
		raise();
		setFocus();
	}
}

void MainWindow::devicePropertiesClicked()
{
	const auto rows = ui.deviceList->selectionModel()->selectedRows();

	if (rows.empty())
	{
		return;
	}

	auto device = ds4Items->getDevice(rows[0].row());
	auto dialog = new DevicePropertiesDialog(this, device);

	const auto appliedSlot = [device] (const DeviceSettings& /*oldSettings*/, const DeviceSettings& newSettings)
	{
		if (!device)
		{
			return;
		}

		device->applySettings(newSettings);
		Program::profileCache.saveSettings(device->macAddress(), newSettings);
	};

	connect(dialog, &DevicePropertiesDialog::settingsChanged, this, appliedSlot);

	dialog->exec();
	delete dialog;
}

void MainWindow::startMinimizedToggled(bool value)
{
	Program::settings.startMinimized = value;
}

void MainWindow::minimizeToTrayToggled(bool value)
{
	Program::settings.minimizeToTray = value;
}

void MainWindow::preferredConnectionChanged(int value)
{
	Program::settings.preferredConnection = ConnectionType::_from_integral(value);
}

void MainWindow::systemTrayShowHide(bool /*checked*/)
{
	toggleHide(QSystemTrayIcon::DoubleClick);
}

void MainWindow::systemTrayExit(bool /*checked*/)
{
	close();
}

void MainWindow::onProfilesLoaded()
{
	registerDeviceNotification();
}

void MainWindow::deviceSelectionChanged(const QItemSelection& selected, const QItemSelection& /*deselected*/) const
{
	ui.pushButton_DeviceProperties->setEnabled(!selected.isEmpty());
}
