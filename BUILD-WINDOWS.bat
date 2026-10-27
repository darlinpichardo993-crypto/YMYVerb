@echo off
chcp 65001 >nul
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0BUILD-WINDOWS.ps1"
if errorlevel 1 (
  echo.
  echo El proceso termino con un error. Lee el mensaje anterior.
  pause
)
