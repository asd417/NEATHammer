@ECHO off
SET _PollingInterval=30

:Start
TIMEOUT /T %_PollingInterval%

nircmd.exe dlg "" "" click yes
GOTO Start
