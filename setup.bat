@echo off
cd dependencies
rmdir /s /q fmt-build
mkdir fmt-build
cd fmt-build

cmake -G "Visual Studio 16 2019" -A "x64" -D "CMAKE_CONFIGURATION_TYPES=Debug;Release" "..\fmt"

cd ..\..
