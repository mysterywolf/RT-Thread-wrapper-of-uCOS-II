# 日志

### 2020-09-17

- 创建工程，实现工程雏形


### 2020-09-18

- 实现开关中断/开关调度器等核心函数


### 2020-09-20

- 实现内存池OS_MEM

- 增加`os_cpu_c.c` `app_hooks.c`钩子函数文件

### 2020-09-26

- 完成OS_TMR的基本功能

### 2020-09-27

- 完成OS_TMR的全部功能，实现带有延迟的周期延时功能

### 2020-09-30

- 实现`OSTaskCreateExt()`函数

### 2020-10-03

- 实现`OSTaskDel()` `OSTaskNameGet()` `OSTaskNameSet()` `OSTaskRegGet()` `OSTaskRegGetID()` `OSTaskRegSet()` `OSTaskDelReq()` `OSTaskQuery()` 函数

### 2020-10-04

- 实现`OSTaskChangePrio` `OSTaskResume` `OSTaskSuspend` `OSTaskStkChk`函数, task.c函数全部完成

### 2020-10-05

- 修复`OSTaskCreateExt`函数堆栈增长方向兼容RT-Thread的问题
- 修复`OSTaskStkChk`函数计算任务堆栈大小错误的问题
- 实现`OSTimeDlyResume`函数
- 实现统计任务以及CPU使用率计算

### 2020-10-10

- 实现`OSSemDel`函数

### 2020-10-11

- 实现`OSSemPend`函数
- 实现`OSSemPost`函数

### 2020-10-12

- 实现`OSSemPendAbort`函数



# 版本

## v0.1.0





# TODO

