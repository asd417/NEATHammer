@ECHO off
SET _PollingInterval=30

:BatchStart
injectory_x86.exe --launch StarCraft.exe --inject bwapi-data\BWAPI.dll wmode.dll

:Start
:: Uncomment the following line on versions of Windows prior to Windows 7 and comment out the TIMEOUT line. The PING solution will not be 100% accurate with _PolingInterval.
:: PING 127.0.0.1 -n %_PollingInterval% >nul
TIMEOUT /T %_PollingInterval%

SET PID=
FOR /F "tokens=2 delims= " %%i IN ('TASKLIST ^| FIND /i "StarCraft.exe"') DO SET PID=%%i
IF [%PID%]==[] (
    ECHO Application was not running. Restarting script.
    GOTO BatchStart
)
GOTO Start
