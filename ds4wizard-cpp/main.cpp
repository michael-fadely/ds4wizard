#include "pch.h"
#include "MainWindow.h"
#include <QApplication>
#include <singleapplication.h>
#include "program.h"

#ifdef QT_IS_BROKEN
#include <Windows.h>
#endif

static MainWindow* window = nullptr;

#ifdef QT_IS_BROKEN

static WNDPROC lpPrevWndFunc = nullptr;

LRESULT CALLBACK windowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (uMsg == WM_DEVICECHANGE)
	{
		MSG msg {};
		msg.hwnd = hwnd;
		msg.message = uMsg;
		msg.wParam = wParam;
		msg.lParam = lParam;

		if (window->wndProc(&msg))
		{
			return TRUE;
		}
	}

	return CallWindowProc(lpPrevWndFunc, hwnd, uMsg, wParam, lParam);
}

#endif

int main(int argc, char** argv)
{
#ifdef Q_OS_WIN
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif

	SingleApplication application(argc, argv, false,
	                              SingleApplication::Mode::ExcludeAppPath | SingleApplication::Mode::User | SingleApplication::Mode::ExcludeAppVersion);

	Program::initialize();
	Program::loadSettings();

	window = new MainWindow();

	QObject::connect(&application, &SingleApplication::instanceStarted, window, [&]()
	{
		window->setWindowState(Qt::WindowActive);
		window->show();
		window->raise();
		window->setFocus();
	});

#ifdef QT_IS_BROKEN
	auto hWnd = reinterpret_cast<HWND>(window->winId());

	lpPrevWndFunc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hWnd, GWLP_WNDPROC));
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&windowProc));
#endif

	int result = application.exec();

	Program::saveSettings();
	delete window;

	return result;
}
