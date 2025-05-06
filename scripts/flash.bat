
@echo off

set project=%1

if "%1" == "" (
    echo "ERROR: no project specified!"
    echo "usage: gdb_start.bat [PROJECT_NAME]"
    exit /b 1
)

:: 检查 ocd.config 是否存在
set ocd_config=projects/%project%/ocd.config
if not exist %ocd_config% (
    echo "ERROR: %ocd_config% not found!"
    exit /b 1
)

:: 读取 ocd.config 并提取 interface 和 target
set if=
set ta=
for /f "tokens=1,2 delims==" %%a in (%ocd_config%) do (
    if "%%a" == "interface" set if=%%b
    if "%%a" == "target" set ta=%%b
)

:: 检查是否成功读取配置
if "%if%" == "" (
    echo "ERROR: 'interface' not found in %ocd_config%!"
    exit /b 1
)
if "%ta%" == "" (
    echo "ERROR: 'target' not found in %ocd_config%!"
    exit /b 1
)

::  这里如果不加 -c "reset halt;wait_halt"^，则第二次下载会报错
:: cmsis-dap stlink
openocd^
 -f interface/%if%.cfg^
 -f target/%ta%.cfg^
 -c init^
 -c "reset halt;wait_halt"^
 -c "flash write_image erase output/%project%/%project%.elf"^
 -c reset^
 -c shutdown



