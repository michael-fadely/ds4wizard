@echo off
setlocal EnableDelayedExpansion

:MAIN
	pushd "%~dp0\dependencies"

	call :MAKE_CMAKE_PROJECT "fmt"
	call :MAKE_CMAKE_PROJECT "ViGEmClient"

	popd
goto :EOF

@rem %~1 = Source directory name (assumes working directory is the dependencies directory)
:MAKE_CMAKE_PROJECT
	if exist "%~1-build" (
		rmdir /s /q "%~1-build"
	)

	mkdir "%~1-build"
	pushd "%~1-build"

	cmake -G "Visual Studio 17 2022" -A "x64" -D "CMAKE_CONFIGURATION_TYPES=Debug;Release" "..\%~1"

	popd
	exit /b %ERRORLEVEL%