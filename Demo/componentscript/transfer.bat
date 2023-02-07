for %%i in (*.lua) do (
	D:\EasyVspServer\luajit  -b %%i %%~ni.out
)
