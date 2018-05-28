#include "stdafx.h"
#include "MainWindow.h"
#include "devicepropertiesdialog.h"
#include "program.h"

// TODO: update Program::settings on checkbox change

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->supportsSystemTray = QSystemTrayIcon::isSystemTrayAvailable();

	if (supportsSystemTray)
	{
		trayIcon = new QSystemTrayIcon(this);
		trayIcon->setIcon(QIcon(":/ds4wizardcpp/Resources/race_q00.ico"));
		trayIcon->show();

		connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		        this, SLOT(toggleHide(QSystemTrayIcon::ActivationReason)));

		if (!Program::settings.startMinimized)
		{
			show();
		}
	}

	ui.checkMinimizeToTray->setChecked(Program::settings.minimizeToTray);
	ui.checkStartMinimized->setChecked(Program::settings.startMinimized);
	ui.comboConnectionType->setCurrentIndex(static_cast<int>(Program::settings.preferredConnection));
}

MainWindow::~MainWindow()
{
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

void MainWindow::closeEvent(QCloseEvent* event)
{
	// reasons
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
