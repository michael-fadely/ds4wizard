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
	ui.setupUi(this);
	this->supportsSystemTray = QSystemTrayIcon::isSystemTrayAvailable();

	if (supportsSystemTray)
	{
		auto menu = new QMenu(this);

		QAction* action = menu->addAction(tr("Show/Hide"));
		connect(action, SIGNAL(triggered(bool)), this, SLOT(systemTrayShowHide(bool)));

		menu->addSeparator();

		action = menu->addAction(tr("&Exit"));
		connect(action, SIGNAL(triggered(bool)), this, SLOT(systemTrayExit(bool)));

		trayIcon = new QSystemTrayIcon(this);
		trayIcon->setIcon(QIcon(":/ds4wizardcpp/Resources/race_q00.ico"));
		trayIcon->setContextMenu(menu);
		trayIcon->show();

		connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		        this, SLOT(toggleHide(QSystemTrayIcon::ActivationReason)));

		if (!Program::settings.startMinimized)
		{
			show();
		}
	}

	Logger::lineLogged += std::bind(&MainWindow::something, this, std::placeholders::_1, std::placeholders::_2);

	ui.checkMinimizeToTray->setChecked(Program::settings.minimizeToTray);
	ui.checkStartMinimized->setChecked(Program::settings.startMinimized);
	ui.comboConnectionType->setCurrentIndex(static_cast<int>(Program::settings.preferredConnection));

	registerDeviceNotification();

	deviceManager = std::make_shared<Ds4DeviceManager>();
	Program::profileCache.setDevices(deviceManager);

	Program::profileCache.load();
	deviceManager->findControllers();
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

void MainWindow::something(void* sender, LineLoggedEventArgs* args) const
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
			title = "Info";
			break;

		case LogLevel::warning:
			icon = QSystemTrayIcon::MessageIcon::Warning;
			title = "Warning";
			break;

		case LogLevel::error:
			icon = QSystemTrayIcon::MessageIcon::Critical;
			title = "Error";
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

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
	auto msg = static_cast<MSG*>(message);

	if (msg->message == WM_DEVICECHANGE && msg->wParam == DBT_DEVICEARRIVAL)
	{
		auto hdr = reinterpret_cast<DEV_BROADCAST_HDR*>(msg->lParam);

		if (!hdr || hdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
		{
			return false;
		}

		auto devinterface = reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE*>(msg->lParam);

		try
		{
			if (Ds4DeviceManager::isDs4(devinterface->dbcc_name))
			{
				// TODO: pull required metadata (instance id) from device and open directly instead of re-scanning everything
				deviceManager->findControllers();
			}
		}
		catch (const std::exception&)
		{
			// HACK: ignored
		}
	}

	return false;
}

void MainWindow::closeEvent(QCloseEvent* event)
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

void MainWindow::systemTrayShowHide(bool checked)
{
	toggleHide(QSystemTrayIcon::DoubleClick);
}

void MainWindow::systemTrayExit(bool checked)
{
	close();
}
