@echo off
setlocal
pushd "%~dp0" || exit /b 1
if not exist "DobryFarmar.exe" goto incomplete
if not exist "Data\Data.pack" goto missingdata
if not exist "Data\cz.pack" goto missingdata
start "" "DobryFarmar.exe" --data Data --common-dir Saves
set "result=%errorlevel%"
popd
exit /b %result%

:incomplete
echo Rozbalte vsechny soubory z vydani do jedne slozky.
echo Extract all files from the release to the same folder.
pause
popd
exit /b 1

:missingdata
echo Zkopirujte slozku Data z puvodni ceske hry vedle Hrat.cmd.
echo Copy the Data folder from your original Czech game next to Hrat.cmd.
echo Required files: Data\Data.pack and Data\cz.pack.
pause
popd
exit /b 1
