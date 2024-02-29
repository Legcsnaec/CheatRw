set "projectpath=%cd%"
cd ../
set "preProjectpath=%cd%"
cd %projectpath%
set "SignFullPath=%preProjectpath%/x64/Debug/DrvLoader.sys"
Build.exe %SignFullPath%