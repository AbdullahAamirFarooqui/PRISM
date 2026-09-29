@echo off
REM ---------------------------------------------------------------------------
REM  PRISM one-click build (Windows + Qt 6 MinGW).
REM  Change QT_DIR / MINGW_DIR below if Qt is installed somewhere else.
REM  Result: build\PRISM.exe (GUI), build\prism_tests.exe, build\prism_console.exe
REM ---------------------------------------------------------------------------
set QT_DIR=C:\Qt\6.8.3\mingw_64
set MINGW_DIR=C:\Qt\Tools\mingw1310_64
set TOOLS=C:\Qt\Tools
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%TOOLS%\CMake_64\bin;%TOOLS%\Ninja;%PATH%

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=%QT_DIR% || goto :error
cmake --build build || goto :error

REM Copy the Qt DLLs next to PRISM.exe so it can be started by double-click.
windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw build\PRISM.exe >nul || goto :error

echo.
echo Running unit tests...
build\prism_tests.exe || goto :error

echo.
echo Build finished. Start the app with:  build\PRISM.exe
goto :eof

:error
echo.
echo BUILD FAILED
exit /b 1
