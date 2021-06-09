/*
 * Copyright (c) 2021, Meco Jianting Man <jiantingman@foxmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-30     Meco Man     first version
 */
#include <ucos_ii.h>

#define TASK_SIZE  256
#define TASK1_PRIO   5
#define TASK2_PRIO   6

static OS_STK task1_stack [TASK_SIZE];
static OS_STK task2_stack [TASK_SIZE];

static OS_EVENT *pMutex;

static void task1(void *p_arg)
{
    INT8U err;

    while(1)
    {
        OSMutexPend(pMutex,0,&err);
        rt_kprintf("task1\n");
        OSMutexPost(pMutex);
        OSTimeDly(1000);
    }
}

static void task2(void *p_arg)
{
    INT8U err;

    while(1)
    {
        OSMutexPend(pMutex,0,&err);
        rt_kprintf("task2\n");
        OSMutexPost(pMutex);
        OSTimeDly(1000);
    }
}

void mutex_example (void)
{
    INT8U err;
    pMutex = OSMutexCreate(0,&err); /*第一个参数在兼容层中没有意义,可以随意填写*/

#if OS_STK_GROWTH == 0u
    OSTaskCreateExt(task1,0,&task1_stack[0],TASK_PRIO,0,&task1_stack[TASK_SIZE-1],TASK_SIZE,0,OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
#else
    OSTaskCreateExt(task1,0,&task1_stack[TASK_SIZE-1],TASK1_PRIO,0,&task1_stack[0],TASK_SIZE,0,OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
#endif

#if OS_STK_GROWTH == 0u
    OSTaskCreateExt(task2,0,&task2_stack[0],TASK_PRIO,0,&task2_stack[TASK_SIZE-1],TASK_SIZE,0,OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
#else
    OSTaskCreateExt(task2,0,&task2_stack[TASK_SIZE-1],TASK2_PRIO,0,&task2_stack[0],TASK_SIZE,0,OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
#endif
}
