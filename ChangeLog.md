# 日志

### 2020-09-17

- 创建工程，实现工程雏形


### 2020-09-18

- 实现开关中断/开关调度器等核心函数


### 2020-09-20

- 实现内存池OS_MEM

- 增加`os_cpu_c.c` `app_hooks.c`钩子函数文件

### 2020-09-26

- 完成`OS_TMR`的基本功能

### 2020-09-27

- 完成`OS_TMR`的全部功能，实现带有延迟的周期延时功能
- 至此`os_tmr.c`文件的函数全部实现

### 2020-09-30

- 实现`OSTaskCreateExt()`函数

### 2020-10-03

- 实现`OSTaskDel()` `OSTaskNameGet()` `OSTaskNameSet()` `OSTaskRegGet()` `OSTaskRegGetID()` `OSTaskRegSet()` `OSTaskDelReq()` `OSTaskQuery()` 函数

### 2020-10-04

- 实现`OSTaskChangePrio()` `OSTaskResume()` `OSTaskSuspend()` `OSTaskStkChk()`函数

### 2020-10-05

- 修复`OSTaskCreateExt()`函数堆栈增长方向兼容RT-Thread的问题
- 修复`OSTaskStkChk()`函数计算任务堆栈大小错误的问题
- 实现`OSTimeDlyResume()`函数
- 实现统计任务以及CPU使用率计算
- 至此`os_task.c`文件的函数全部实现

### 2020-10-10

- 实现`OSSemDel()`函数

### 2020-10-11

- 实现`OSSemPend()`函数
- 实现`OSSemPost()`函数

### 2020-10-12

- 实现`OSSemPendAbort()` `OSSemSet()` ``OSSemAccept()`` `OSSemQuery()`函数
- 至此`os_sem.c`文件的函数全部实现

### 2020-10-16

- 实现`PKG_USING_UCOSII_WRAPPER_AUTOINIT`宏定义
- 实现`OSMutexCreate()`函数

### 2020-10-24

- 实现`OSMutexPend()` `OSMutexPost()` 函数
- 增加`mutex_example.c`文件

### 2020-10-25

- 实现`OSMutexDel()` `OSMutexAccept()` `OSMutexQuery()`函数
- 至此`os_mutex.c`文件函数全部实现

### 2020-11-01

- 额外实现`OSMutexCreateEx()`函数，该函数并不在uCOS-II原版的函数中，`OSMutexCreate()`函数中第一个参数`prio`在兼容层中没有任何意义，因此该函数将`OSMutexCreate()`函数中的第一个参数略去，以方便用户使用。原因是由于uCOS-II的实现方式过于落后，不支持相同任务在同一优先级。
- 实现`OSQCreate()`、`OSQPend()`、`OSQPost()`、`OSQPostFront()`函数
- 额外实现``OSQCreateEx()`函数，该函数并不在uCOS-II原版的函数中，`OSQCreateEx()`函数中第一个参数`size`在本兼容层中没有意义，因此该函数将`OSQCreateEx()`函数中的第一个参数略去，以方便用户使用。
- 增加`messagequeue_example.c`示例文件

### 2020-11-02

- 实现`OSQDel()`、`OSQFlush()` 函数

### 2020-11-08

- 实现`OSQAccept()`、`OSQQuery()`、`OSQPostOpt()`函数

### 2020-11-14

- 头文件部分包含由<>改为""
- 实现`OSQPendAbort`函数

### 2020-11-24

- 将自动初始化由`INIT_DEVICE_EXPORT`提前至`INIT_PREV_EXPORT`

### 2020-11-27

- 调整`os_q.c`、`os_mutex.c`、`os_sem.c`调整获取内核对象指针语句的顺序，防止出现野指针瞎指
- 实现`OSQPostOpt()`函数广播机制

### 2020-11-28

- 实现`os_mbox.c`文件所有函数
- 整理`ucos_ii.h`宏定义，并精简了其他文件的宏定义，删除了没有必要的宏定义
- 调整keil工程文件结构和顺序

### 2020-11-29

- 增加SConscript脚本

### 2020-12-11

- 实现`OSFlagCreate()`、`OSFlagPendGetFlagsRdy()`、`OSFlagQuery()`函数


### 2020-12-12

- 完成事件标志组的兼容

### 2020-12-16

- 修复`OSTmrRemainGet()`函数返回结果单位不一致的问题
- 修复`OS_TMR`结构体`.OSTmrMatch`成员变量数值单位不一致的问题
- 修复从RT-Thread到uCOS-II定时器换算公式没有考虑到Systick变化的问题



# Release

## v0.1.0

已经实现了uCOS-II的兼容，发布第一个版本



# TODO

OSTCBCyclesTot OSTCBCyclesStart