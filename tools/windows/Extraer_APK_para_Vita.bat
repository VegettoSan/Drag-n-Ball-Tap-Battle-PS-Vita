@echo off
setlocal DisableDelayedExpansion
title Dragon Ball Tap Battle - Preparar datos para PS Vita
set "DBTB_TOOL=%~dp0Extraer_APK_para_Vita.ps1"
set "DBTB_OUTPUT=%~dp0Listo_para_Vita"
set "DBTB_ARG_COUNT=0"
:collect
if "%~1"=="" goto run
set "DBTB_APK_%DBTB_ARG_COUNT%=%~f1"
set /a DBTB_ARG_COUNT+=1 >nul
shift
goto collect
:run
if not exist "%DBTB_TOOL%" goto missing
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%DBTB_TOOL%" -FromLauncher
set "DBTB_RESULT=%errorlevel%"
goto finish
:missing
echo ERROR: falta Extraer_APK_para_Vita.ps1 al lado de este BAT.
echo Extrae primero todos los archivos del ZIP en una carpeta.
set "DBTB_RESULT=2"
:finish
if "%DBTB_NO_PAUSE%"=="1" goto done
echo.
pause
:done
exit /b %DBTB_RESULT%
