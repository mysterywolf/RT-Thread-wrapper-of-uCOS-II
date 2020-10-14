/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-17     Meco Man     first version
 */

/*本文件展示了兼容层实现的uCOS-II官方给出的标准初始化流程*/
/*启动以及初始化过程严格遵照官方给出的例程*/
/*串口使用USART2*/

#include <ucos_ii.h>

void task_example (void);
void timer_example (void);
void sem_example (void);

int main(void)/*RT-Thread main线程*/
{
    OSInit();    
    OSStart();
    
#if OS_TASK_STAT_EN > 0u
    OSStatInit();
#endif
    
//    task_example();
//    timer_example(); 
    sem_example();    
}
