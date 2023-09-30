@ECHO OFF
SETLOCAL

@REM Run the powershell script with the same name as this bat file

PowerShell -NoProfile -ExecutionPolicy Bypass -Command "& '%~dpn0.ps1' %*"

ENDLOCAL