@ECHO off
SET _PollingInterval=30

:BatchStart
injectory_x86.exe --launch StarCraft.exe --inject bwapi-data\BWAPI.dll wmode.dll

:Start
TIMEOUT /T %_PollingInterval%

SET PID=
FOR /F "tokens=2 delims= " %%i IN ('TASKLIST ^| FIND /i "StarCraft.exe"') DO SET PID=%%i
IF [%PID%]==[] (
    ECHO Application was not running. Restarting script.
    GOTO BatchStart
)
GOTO Start
