#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "program.h"

#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

// TODO: single instance https://stackoverflow.com/a/26904110

#ifdef QT_IS_FUCKING_BROKEN
#include <Windows.h>
#endif

static MainWindow* window = nullptr;

#ifdef QT_IS_FUCKING_BROKEN

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
	CoInitialize(nullptr);
	QApplication application(argc, argv);

	Program::initialize();
	Program::loadSettings();

	window = new MainWindow();

#ifdef QT_IS_FUCKING_BROKEN
	auto hWnd = reinterpret_cast<HWND>(window->winId());

	lpPrevWndFunc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hWnd, GWLP_WNDPROC));
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&windowProc));
#endif

	int result = application.exec();

	Program::saveSettings();
	delete window;

	return result;
}
