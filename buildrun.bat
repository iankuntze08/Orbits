@echo off
set /p dec= "Clear CMake cache? (y/N): "
if /I "%dec%"=="y" (del build)

cmake . -B build
cmake --build build
.\build\Debug\GLVSCode.exe %*