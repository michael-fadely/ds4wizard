#include "stdafx.h"
#include "ds4wizardcpp.h"
#include "devicepropertiesdialog.h"

ds4wizardcpp::ds4wizardcpp(QWidget* parent)
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
		        this, SLOT(toggle_hide(QSystemTrayIcon::ActivationReason)));
	}
}

ds4wizardcpp::~ds4wizardcpp()
{
	delete trayIcon;
}

void ds4wizardcpp::changeEvent(QEvent* e)
{
	switch (e->type())
	{
		case QEvent::WindowStateChange:
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


void ds4wizardcpp::toggleHide(QSystemTrayIcon::ActivationReason reason)
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
	trayIcon->showMessage("hi", "this is a test");
	auto dialog = new DevicePropertiesDialog();
	dialog->exec();
	delete dialog;
}
