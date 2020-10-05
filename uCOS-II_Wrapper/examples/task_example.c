#include <ucos_ii.h>

#define TASK_SIZE  256
#define TASK_PRIO   5

static OS_STK task_stack [TASK_SIZE];

static void task(void *p_arg)
{
    OS_STK_DATA stk;
    while(1)
    {
        OSTaskStkChk(OS_PRIO_SELF, &stk);
        rt_kprintf("task stack free: %d\r\n", stk.OSFree);
        rt_kprintf("CPU Usage:%d%%\r\n",OSCPUUsage);
        rt_thread_delay(500);
    }
}

void task_example (void)
{
#if OS_STK_GROWTH == 0u      
    OSTaskCreateExt(task,0,&task_stack[0],TASK_PRIO,0,&task_stack[TASK_SIZE-1],TASK_SIZE,0,OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
#else
    OSTaskCreateExt(task,0,&task_stack[TASK_SIZE-1],TASK_PRIO,0,&task_stack[0],TASK_SIZE,0,OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
#endif
}
