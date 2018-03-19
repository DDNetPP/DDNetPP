@echo off
title ddpp compiler windows
::
color 7c

set /p release=Type: 

if exist "%VCINSTALLDIR%" (
	goto compile
)
if exist "%VS140COMNTOOLS%" (
	set VSPATH="%VS140COMNTOOLS%"
	goto set_env
)
if exist "%VS110COMNTOOLS%" (
	set VSPATH="%VS110COMNTOOLS%"
	goto set_env
)
if exist "%VS100COMNTOOLS%" (
	set VSPATH="%VS100COMNTOOLS%"
	goto set_env
)
if exist "%VS90COMNTOOLS%" (
	set VSPATH="%VS90COMNTOOLS%"
	goto set_env
)
if exist "%VS80COMNTOOLS%" (
	set VSPATH="%VS80COMNTOOLS%"
	goto set_env
)

echo You need Microsoft Visual Studio 8, 9 or 10 installed
pause
exit

:set_env
if not exist %VSPATH%vsvars32.bat (
	color 0a
	cls
	echo.
	echo === An error occured! ===
	echo.
	pause >nul
	exit
) else call %VSPATH%vsvars32.bat

:: Compile
:compile
echo.
@echo === Building Teeworlds %release% ===
@call bam.exe %release% -j 8
@echo === Finished === last: %time%
echo.

:: Ending/Or not :D
@echo Press any Key to Compile again...
@pause >nul
color 0a
@echo === Again :D ===
color 7c
cls
goto compile
