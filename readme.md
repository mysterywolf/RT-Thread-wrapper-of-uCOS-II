# RT-Thread操作系统的μC/OS-II兼容层

## 让基于μC/OS-II开发的应用层无感地迁移到RT-Thread操作系统

中文 | English

_____________

[TOC]

# 0 前排提示

本文含有图片，受限于**中国大陆互联网环境**，访问github时，**readme.md(本文件)的图片一般加载不出来**，因此我导出了.pdf文件。如果您需要详细阅读，可以将项目下载或clone下来，阅读[docs/中文说明文档.pdf](docs/中文说明文档.pdf)文件。

**如果你喜欢本项目，请点击右上角的Star予以支持，开源项目的成就感就靠star了，谢谢！**





# 1 概述

这是一个针对RT-Thread国产操作系统的μCOS-II操作系统兼容层，可以让基于美国Micriμm公司的μCOS-II操作系统的项目快速、无感地迁移到RT-Thread操作系统上。在兼容层的设计、编写上尊重原版μC/OS-II，保证原版μC/OS-II的原汁原味。

支持版本：μC/OS-II 2.00-2.93全部版本

RT-Thread操作系统的μCOS-III兼容层：https://github.com/mysterywolf/RT-Thread-wrapper-of-uCOS-III



## 1.1 本兼容层适合于：

- 之前学习过μCOS-II操作系统，意图转向学习RT-Thread国产操作系统。本兼容层可以帮您用已有的μCOS-II编程经验和习惯快速将项目跑起来，日后在应用过程中深入熟悉RT-Thread的API函数，逐步向RT-Thread过度，降低您的学习门槛和时间成本。**有了本兼容层，对RT-Thread API以及编程风格的不熟悉再也不是您学习RT-Thread的阻力！**

- 现有任务（线程）模块采用μCOS-II编写，想要用在基于RT-Thread的工程上

- 老项目需要从μCOS-II操作系统向RT-Thread操作系统迁移

- 当需要快速基于RT-Thread开发产品，但是工程师之前均采用μC/OS开发，从未用过RT-Thread的开发经验。本兼容层可以帮助让工程师快速基于μC/OS-II开发经验开发产品，简化软件的重用、缩短微控制器新开发人员的学习过程，并缩短新设备的上市时间。

- 避免在从μCOS-II迁移到RT-Thread时，由于μCOS-II的编程经验导致的思维定式引发的错误，这种错误一般很难被发现 

  > ​    例如：
  >
  > 1. 软件定时器参数的不同
  >
  > 2. 任务堆栈的数据类型不同



## 1.2 版本详细信息

|    组件名称    | 版本号  |         配置文件          |                说明                 |
| :------------: | :-----: | :-----------------------: | :---------------------------------: |
| RT-Thread nano |  3.1.3  |        rtconfig.h         |                                     |
|    μC/OS-II    | 2.93.00 | os_cfg.h<br />app_hooks.c | 兼容层兼容2.00-2.93全部μCOS-III版本 |



## 1.3 官网

RT-Thread：https://www.rt-thread.org/   
文档中心：https://www.rt-thread.org/document/site/tutorial/nano/an0038-nano-introduction/

μCOS-II：https://www.micrium.com/  
文档中心：[µC/OS-II - µC/OS-II Documentation - Micrium Documentation](https://doc.micrium.com/pages/viewpage.action?pageId=10753158)





# 2 使用

## 2.1 Keil-MDK仿真工程

本仿真工程是基于STM32F103平台。

Keil工程路径：[rt-thread-3.1.3/bsp/stm32f103/Project.uvprojx](rt-thread-3.1.3/bsp/stm32f103/Project.uvprojx)

需要提前安装好RT-Thread Nano-3.1.3 [Keil支持包](https://www.rt-thread.org/download/mdk/RealThread.RT-Thread.3.1.3.pack).

**注意：调试串口使用的是USART2，不是USART1**

<img src="C:/Users/92036/Desktop/ucosiii/docs/pic/usart2.png" alt="usart2"  />



## 2.2 迁移步骤

**（如果使用的是RT-Thread Nano版请参见以下步骤；若使用RT-Thread完整版可以直接跳转至[Env工具自动化配置到工程中](#6 Env工具自动化配置到工程中)章节）**

1. 将uCOS-II文件夹内的所有文件都加入到你的工程中，最好保持原有文件夹的结构。相较于原版μCOS-II增加了`os_rtwrap.c`文件，负责对RT-Thread和μCOS-II的转换提供支持。
2. 配置`os_cfg.h`  
   每个选项的配置说明和原版μCOS-II一致，若有不同，我已经在注释中有所解释。  
   **原版μCOS-II配置**说明可参见：  
   a)《嵌入式实时操作系统μC/OS-II（第二版）》北京航空航天大学出版社 邵贝贝等译   
   b) Micriμm公司μCOS-II[在线文档](https://doc.micrium.com/pages/viewpage.action?pageId=16879637)
3. μCOS-II原版定时器回调函数是在定时器线程中调用的，而非在中断中调用，因此要使用μCOS-II兼容层的软件定时器，需要将rtconfig.h中的宏定义`RT_USING_TIMER_SOFT`置1。
4. 由于兼容层采用rt-thread内核自带的堆内存分配方式，因此免去了原版uCOS-II中配置任务以及各内核对象内存池大小的步骤，遂需要在rtconfig.h中定义`RT_USING_MEMHEAP`



## 2.3 os_cfg.h配置文件

由于兼容层采用rt-thread内核自带的堆内存分配方式，因此免去了原版uCOS-II中配置任务以及各内核对象内存池大小的步骤，遂上述相关宏定义在兼容层中**均被删除**，涉及到：

```c

```



## 2.4 运行

### 2.4.1 手动初始化流程

本兼容层完全兼容官方给出的标准初始化流程，如果您兼容老项目，μCOS-III初始化部分无需做任何修改。



### 2.4.2 自动初始化流程

如果您在应用层中不想手动初始化本兼容层，可以在`rtconfig.h`文件中定义`PKG_USING_UCOSII_WRAPPER_AUTOINIT`宏定义。请参见 [6.2.1章节](#6.2.1 Enable uCOS-III wrapper automatically init)（**如无特殊要求，建议采用该种方式**）。



## 2.5 注意

1. μCOS-II的任务堆栈大小单位是`sizeof(CPU_STK)`，而RT-Thread的线程堆栈大小单位是`sizeof(rt_uint8_t)`，虽然在兼容层已经做了转换，但是在填写时一定要注意，所有涉及到μCOS-II的API、宏定义全部是按照μCOS-II的标准，即堆栈大小为`sizeof(CPU_STK)`，**切勿混搭**！这种错误极其隐晦，一定要注意！**下面是混搭的错误示例**：</br>

   ```c
   ALIGN(RT_ALIGN_SIZE)
   static rt_uint8_t thread2_stack[1024];//错误：混搭RT-Thread的数据类型定义线程堆栈
   
   OSTaskCreateExt(task,
                   0,
                   &task_stack[TASK_SIZE-1],
                   TASK_PRIO,
                   0,
                   &task_stack[0],
                   sizeof(thread2_stack),//任务堆栈大小(错误：这个参数的单位是sizeof(CPU_STK))
                   0,
                   OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
   ```

   **下面是正确写法**：</br>

   ```c
   #define THREAD_STACK_SIZE       256 //正确，要通过宏定义单独定义堆栈大小，单位为sizeof(CPU_STK)
   ALIGN(RT_ALIGN_SIZE)
       static CPU_STK thread2_stack[THREAD_STACK_SIZE];//正确，使用uCOS-III自己的数据类型定义任务堆栈
   
   OSTaskCreateExt(task,
                   0,
                   &task_stack[TASK_SIZE-1],
                   TASK_PRIO,
                   0,
                   &task_stack[0],
                   THREAD_STACK_SIZE,//任务堆栈大小(正确)
                   0,
                   OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
   ```

2. 本封装层文件内含有中文注释，编码格式**ANSI - GB2312**，并非UTF-8编码。





# 3 接口



















# 7 友情链接

## 7.1 RT-Thread Nano移植教程

官方文档：

> https://www.rt-thread.org/document/site/tutorial/nano/an0038-nano-introduction/

视频教程：

> 基于 MDK 移植 RT-Thread Nano：https://www.bilibili.com/video/BV1TJ411673o
>
> 基于 IAR 移植 RT-Thread Nano：https://www.bilibili.com/video/BV1BJ41177CW
>
> 基于 CubeMX 移植 RT-Thread Nano：https://www.bilibili.com/video/BV1KJ41167qg



## 7.2 RT-Thread FinSH控制台教程

官方文档：

> https://www.rt-thread.org/document/site/programming-manual/finsh/finsh/

视频教程：

> https://www.bilibili.com/video/BV1r741137sY?p=1





# 8 其他

## 8.1 联系方式

维护：[Meco Man](https://github.com/mysterywolf/)

联系方式：jiantingman@foxmail.com



## 8.2 主页

> https://github.com/mysterywolf/RT-Thread-wrapper-of-uCOS-II 
>
> https://gitee.com/mysterywolf/RT-Thread-wrapper-of-uCOS-II （国内镜像，定时同步）



## 8.3 开源协议

采用 Apache-2.0 开源协议，细节请阅读项目中的 LICENSE 文件内容。



## 8.4 支持

如果您喜欢本项目**可以在本页右上角点一下Star**，可以赏我五毛钱，用以满足我小小的虚荣心，并激励我继续维护好这个项目。

<img src="C:/Users/92036/Desktop/ucosiii/docs/pic/donate.png" style="zoom: 67%;" />