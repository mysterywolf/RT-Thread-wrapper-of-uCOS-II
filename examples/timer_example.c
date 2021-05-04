/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-30     Meco Man     first version
 */
#include <ucos_ii.h>

static void tmr_callback (void *ptmr, void *parg)
{
    rt_kprintf("tmr\r\n");
}

void timer_example (void)
{
    OS_TMR* tmr1;
    INT8U err;

    tmr1=OSTmrCreate(30,10,OS_TMR_OPT_PERIODIC,tmr_callback,0,(INT8U*)"tmr",&err);
    OSTmrStart(tmr1,&err);
}
