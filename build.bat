@echo off

if not exist build\bld.exe (
    mkdir build
    cd build
    cl -Fe:bld.exe ../src/build_tool/main.cpp -FC -WX -W3 -wd4200 -wd4244 -diagnostics:column -nologo -Zi -D_CRT_SECURE_NO_WARNINGS
    cd ..
)

rem ubuntu run ./build.sh
build\bld.exe --quick
