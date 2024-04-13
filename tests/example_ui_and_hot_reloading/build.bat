@echo off
mkdir build
cd build

clang unity_exe.c -o platform.exe -g -O0 -I"../.."
clang unity_dll.c -o game.dll -O0 -shared -g -I"../.." -Wl,-export:APP_Update

