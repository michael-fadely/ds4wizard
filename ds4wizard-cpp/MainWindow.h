#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "Ds4DeviceManager.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

	void changeEvent(QEvent* e) override;

private:
	std::shared_ptr<Ds4DeviceManager> deviceManager;

	Ui::MainWindow ui;
	bool supportsSystemTray = false;
	QSystemTrayIcon* trayIcon = nullptr;
	PVOID notificationHandle = nullptr;

	void registerDeviceNotification();
	void unregisterDeviceNotification();
	bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;

protected slots:
	void closeEvent(QCloseEvent* event) override;
	void toggleHide(QSystemTrayIcon::ActivationReason reason);
	void devicePropertiesClicked() const;
	static void startMinimizedToggled(bool value);
	static void minimizeToTrayToggled(bool value);
	static void preferredConnectionChanged(int value);
	void systemTrayShowHide(bool checked);
	void systemTrayExit(bool checked);
};
