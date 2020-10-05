#include <ucos_ii.h>

#define TASK_SIZE  256
#define TASK_PRIO   5

static OS_STK task_stack [TASK_SIZE];

static void task(void *p_arg)
{
    while(1)
    {
        rt_kprintf("TASK\r\n");
        rt_thread_delay(500);
    }
}

void task_example (void)
{
#if OS_STK_GROWTH == 0u      
    OSTaskCreateExt(task,0,&task_stack[0],TASK_PRIO,0,&task_stack[TASK_SIZE-1],TASK_SIZE,0,0);
#else
    OSTaskCreateExt(task,0,&task_stack[TASK_SIZE-1],TASK_PRIO,0,&task_stack[0],TASK_SIZE,0,0);
#endif
}
