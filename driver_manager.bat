@echo off
REM SSAPT Driver Management Script
REM Requires Administrator privileges

setlocal

set DRIVER_NAME=SSAPT
set DRIVER_PATH=%~dp0ssapt.sys

if "%1"=="" goto usage
if "%1"=="install" goto install
if "%1"=="start" goto start
if "%1"=="stop" goto stop
if "%1"=="uninstall" goto uninstall
if "%1"=="status" goto status
goto usage

:install
echo Installing SSAPT driver...
sc create %DRIVER_NAME% binPath= "%DRIVER_PATH%" type= kernel start= demand
if %errorlevel% neq 0 (
    echo Failed to install driver
    exit /b 1
)
echo Driver installed successfully
goto end

:start
echo Starting SSAPT driver...
sc start %DRIVER_NAME%
if %errorlevel% neq 0 (
    echo Failed to start driver
    exit /b 1
)
echo Driver started successfully
goto end

:stop
echo Stopping SSAPT driver...
sc stop %DRIVER_NAME%
if %errorlevel% neq 0 (
    echo Failed to stop driver
    exit /b 1
)
echo Driver stopped successfully
goto end

:uninstall
echo Uninstalling SSAPT driver...
sc stop %DRIVER_NAME% >nul 2>&1
sc delete %DRIVER_NAME%
if %errorlevel% neq 0 (
    echo Failed to uninstall driver
    exit /b 1
)
echo Driver uninstalled successfully
goto end

:status
echo Querying SSAPT driver status...
sc query %DRIVER_NAME%
goto end

:usage
echo SSAPT Driver Management
echo =======================
echo.
echo Usage: %~nx0 [command]
echo.
echo Commands:
echo   install   - Install the SSAPT kernel driver
echo   start     - Start the SSAPT driver
echo   stop      - Stop the SSAPT driver
echo   uninstall - Uninstall the SSAPT driver
echo   status    - Check driver status
echo.
echo Note: This script requires Administrator privileges
goto end

:end
endlocal
