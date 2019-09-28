@echo off
cd dependencies
rmdir /s /q fmt-build
mkdir fmt-build
cd fmt-build

cmake -G "Visual Studio 15 2017 Win64" -D "CMAKE_CONFIGURATION_TYPES=Debug;Release" "..\fmt"

cd ..\..
