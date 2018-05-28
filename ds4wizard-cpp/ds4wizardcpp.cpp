#include "stdafx.h"
#include "ds4wizardcpp.h"
#include "devicepropertiesdialog.h"

ds4wizardcpp::ds4wizardcpp(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	this->supports_systray = QSystemTrayIcon::isSystemTrayAvailable();

	if (supports_systray)
	{
		tray_icon = new QSystemTrayIcon(this);
		tray_icon->setIcon(QIcon(":/ds4wizardcpp/Resources/race_q00.ico"));
		tray_icon->show();
		connect(tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		        this, SLOT(toggle_hide(QSystemTrayIcon::ActivationReason)));
	}
}

ds4wizardcpp::~ds4wizardcpp()
{
	delete tray_icon;
}

void ds4wizardcpp::changeEvent(QEvent* e)
{
	switch (e->type())
	{
		case QEvent::WindowStateChange:
			if (windowState() & Qt::WindowMinimized)
			{
				if (supports_systray && ui.checkMinimizeToTray->isChecked())
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


void ds4wizardcpp::toggle_hide(QSystemTrayIcon::ActivationReason reason)
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

void ds4wizardcpp::on_pushButton_DeviceProperties_clicked() const
{
	tray_icon->showMessage("hi", "this is a test");
	auto dialog = new DevicePropertiesDialog();
	dialog->exec();
	delete dialog;
}
