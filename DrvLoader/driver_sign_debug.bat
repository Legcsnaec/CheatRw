set "projectpath=%cd%"
cd ../
set "preProjectpath=%cd%"
cd %projectpath%
set "SignPath=%preProjectpath%\X64\DEBUG\DrvLoader.sys"

::加VMP
::set "SignFullPath=%preProjectpath%/X64/DEBUG/DrvLoader.sys.vmp"
::"D:\VMProtect Ultimate\VMProtect_Con.exe" %SignFullPath%

::破解版签名无需设置系统时间
::set "d=%date:~0,10%"
::date 2015/8/15
"D:\Program Files (x86)\DSignTool\CSignTool.exe" sign /r QihuSign /f %SignPath% /ac
::date %d%