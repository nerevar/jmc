@setlocal enableextensions
@cd /d "%~dp0"

%WinDir%\system32\regsvr32.exe ttcoreex.dll
%WinDir%\system\regsvr32.exe ttcoreex.dll
