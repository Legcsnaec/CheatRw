# CheatRw

​	最终项目生成的是一个exe（用于测试）或者dll（用于发版），此处皆称呼为exe，此exe在其.data段已经存储了驱动文件的二进制数据，此驱动文件为"加载器驱动"，用于内存加载"读写驱动"，"读写驱动"同样存储在"加载器驱动"的.data段，直接在0环申请内存后将"读写驱动"的数据拷贝过去，修复数据后直接call过去，这样就完成了内存加载。

​	在运行exe时首先先启动"加载器驱动"，将.data段的二进制数据转存成本地文件（随机名称），运行完"加载器驱动"后立马删除注册表信息、本地驱动文件。这样就形成无文件落地而且让"读写驱动"执行。（秉持着一个原则就是驱动文件尽量不要落地，因为落地就容易被查）

​	最后，此项目除了没有加VMP壳所有功能皆已完成且测试，目标机器为Win10 21H2 19044、Win7 x64，如果想用其他版本机型进行测试（如Win 10 1909），**需更改"挂起线程"、"恢复线程"这两个接口**，因为这两接口是通过特征码定位的，我并没有去做所有系统的特征码匹配。

编译注意：请更改所使用的自动签名bat脚本中应用路径和对应签名规则，如driver_sign_debug.bat文件说明：

```bash
set "projectpath=%cd%"
cd ../
set "preProjectpath=%cd%"
cd %projectpath%
set "SignPath=%preProjectpath%\X64\DEBUG\DrvLoader.sys"

::破解版签名无需设置系统时间
::set "d=%date:~0,10%"
::date 2015/8/15
:: 签名程序自己路径 sign /r 自己的签名规则 /f %SignPath% /ac
"D:\Program Files (x86)\DSignTool\CSignTool.exe" sign /r QihuSign /f %SignPath% /ac
::date %d%
```

