#include "stdafx.h"
#include "MainWindow.h"
#include "devicepropertiesdialog.h"
#include "DeviceProfileCache.h"
#include "program.h"
#include "Ds4DeviceManager.h"
#include "Logger.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	const auto now = []() -> auto { return std::chrono::high_resolution_clock::now(); };
	const auto start = now();

	ui.setupUi(this);

	const auto elapsed = now() - start;

	qInfo() << __FUNCTION__ " setupUi: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
		<< " ms";

	ui.checkMinimizeToTray->setChecked(Program::settings.minimizeToTray);
	ui.checkStartMinimized->setChecked(Program::settings.startMinimized);
	ui.comboConnectionType->setCurrentIndex(static_cast<int>(Program::settings.preferredConnection));

	this->supportsSystemTray = QSystemTrayIcon::isSystemTrayAvailable();

	if (supportsSystemTray)
	{
		auto loadIconTask = std::async(std::launch::async, [now]() -> auto
		{
			auto start = now();
			QIcon icon(":/ds4wizardcpp/Resources/race_q00.ico");
			auto elapsed = now() - start;
			qInfo() << "icon load time: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
			return icon;
		});

		auto menu = new QMenu(this);

		QAction* action = menu->addAction(tr("Show/Hide"));
		connect(action, SIGNAL(triggered(bool)), this, SLOT(systemTrayShowHide(bool)));

		menu->addSeparator();

		action = menu->addAction(tr("&Exit"));
		connect(action, SIGNAL(triggered(bool)), this, SLOT(systemTrayExit(bool)));

		trayIcon = new QSystemTrayIcon(this);
		trayIcon->setContextMenu(menu);

		connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		        this, SLOT(toggleHide(QSystemTrayIcon::ActivationReason)));

		trayIcon->setIcon(loadIconTask.get());
		trayIcon->show();
	}

	if (!supportsSystemTray || !Program::settings.startMinimized)
	{
		show();
	}

	Logger::lineLogged += [this](auto a, auto b) -> void { onLineLogged(a, b); };

	deviceManager = std::make_shared<Ds4DeviceManager>();
	Program::profileCache.setDevices(deviceManager);

	connect(this, SIGNAL(s_onDeviceOpened(DeviceOpenedEventArgs*)), this, SLOT(onDeviceOpened(DeviceOpenedEventArgs*)));
	connect(this, SIGNAL(s_onDeviceClosed(DeviceClosedEventArgs*)), this, SLOT(onDeviceClosed(DeviceClosedEventArgs*)));
	connect(this, SIGNAL(s_onProfilesLoaded()), this, SLOT(onProfilesLoaded()));

	deviceManager->deviceOpened += [this](void*, DeviceOpenedEventArgs* a) -> void { emit s_onDeviceOpened(a); };
	deviceManager->deviceClosed += [this](void*, DeviceClosedEventArgs* a) -> void { emit s_onDeviceClosed(a); };

	startupTask = std::async(std::launch::async, [this]() -> void
	{
		Program::profileCache.load();
		emit s_onProfilesLoaded();
		deviceManager->findControllers();
	});
}

MainWindow::~MainWindow()
{
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
					QTimer::singleShot(0, this, SLOT(hide()));
				}
			}
			break;

		default:
			break;
	}

	QMainWindow::changeEvent(e);
}

void MainWindow::onLineLogged(void* /*sender*/, LineLoggedEventArgs* args) const
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
		QMessageBox::warning(this, tr("Warning"), tr("Failed to register device notification event."));
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

void MainWindow::populateProfileList() const
{
	ui.profileList->clear();

	auto& profiles_lock = Program::profileCache.profiles_lock;

	{
		lock(profiles);

		for (const DeviceProfile& profile : Program::profileCache.profiles)
		{
			ui.profileList->addItem(QString::fromStdString(profile.name));
		}
	}
}

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

void MainWindow::devicePropertiesClicked() const
{
	trayIcon->showMessage("hi", "this is a test");
	auto dialog = new DevicePropertiesDialog();
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

void MainWindow::onDeviceOpened(DeviceOpenedEventArgs* /*a*/)
{
}

void MainWindow::onDeviceClosed(DeviceClosedEventArgs* /*a*/)
{
}

void MainWindow::onProfilesLoaded()
{
	registerDeviceNotification();
	populateProfileList();
}
