#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "Ds4DeviceManager.h"
#include "Logger.h"
#include "Ds4ItemModel.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

	void changeEvent(QEvent* e) override;
	bool wndProc(tagMSG* msg) const;

private:
	std::shared_ptr<Ds4DeviceManager> deviceManager;
	Ds4ItemModel* ds4Items;

	Ui::MainWindow ui;
	bool supportsSystemTray = false;
	QSystemTrayIcon* trayIcon = nullptr;
	PVOID notificationHandle = nullptr;
	std::future<void> startupTask;
	void onLineLogged(void* sender, std::shared_ptr<LineLoggedEventArgs> args) const;

	void registerDeviceNotification();
	void unregisterDeviceNotification();
	void doFindControllerThing(const std::wstring& name) const;
	void populateProfileList() const;

protected:

#if !defined(QT_IS_FUCKING_BROKEN)
	bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
#endif

protected slots:
	void closeEvent(QCloseEvent* event) override;
	void toggleHide(QSystemTrayIcon::ActivationReason reason);
	void devicePropertiesClicked();
	static void startMinimizedToggled(bool value);
	static void minimizeToTrayToggled(bool value);
	static void preferredConnectionChanged(int value);
	void systemTrayShowHide(bool checked);
	void systemTrayExit(bool checked);

	void onProfilesLoaded();
	void deviceSelectionChanged() const;

signals:
	void s_onProfilesLoaded();
};
