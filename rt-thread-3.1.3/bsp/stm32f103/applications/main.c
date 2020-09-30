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

#include <os.h>

void timer_example (void);

OS_STK task_stack [256];
void   task(void *p_arg)
{
    while(1)
    {
//        rt_kprintf("hahha");
        rt_thread_delay(500);
    }
}

int main(void)/*RT-Thread main线程*/
{
    OSInit();
    
    OSStart();
    
    timer_example();
    OSTaskCreateExt(task,0,task_stack,5,0,0,256,0,0);
}
