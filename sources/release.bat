@echo off

SET release=jmc%1

mkdir %release%
copy jmc.exe %release%
copy ttcoreex.dll %release%
copy recore.dll %release%
copy zlib.dll %release%
copy wolfssl.dll %release%
copy html.log.template %release%
copy language.ini %release%
copy changelog.txt %release%
copy ttcoreex.bat %release%

"c:\Program Files\7-Zip\7z.exe" a %release%.zip %release%

rmdir /s/q %release%
