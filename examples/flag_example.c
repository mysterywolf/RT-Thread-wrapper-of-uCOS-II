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

static OS_FLAG_GRP* pFlag;

static void task1(void *p_arg)
{
    INT8U err;

    while(1)
    {
        OSFlagPost(pFlag, 0x03, OS_FLAG_SET, &err);
        OSTimeDly(1000);
    }
}

static void task2(void *p_arg)
{
    INT8U err;

    while(1)
    {
        OSFlagPend(pFlag, 0x03, OS_FLAG_WAIT_SET_ALL|OS_FLAG_CONSUME, 0, &err);
        if(err == OS_ERR_NONE)
        {
            rt_kprintf("OSFlagPend ok\r\n");
        }
        else
        {
            rt_kprintf("OSFlagPend err\r\n");
        }
    }
}


void flag_example(void)
{
    INT8U err;

    pFlag = OSFlagCreate(0, &err);

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
