#include <ucos_ii.h>

#define TASK_SIZE  256
#define TASK1_PRIO   5
#define TASK2_PRIO   6

static OS_STK task1_stack [TASK_SIZE];
static OS_STK task2_stack [TASK_SIZE];

static OS_EVENT *p_mq;

static void task1(void *p_arg)
{
    while(1)
    {
        OSQPostFront(p_mq, "Hahahaha");
        OSTimeDly(1000);
    }
}

static void task2(void *p_arg)
{
    INT8U err;

    while(1)
    {
        rt_kprintf("get a message: %s\n", OSQPend(p_mq, 0, &err));
    }
}

void messagequeue_example (void)
{
    p_mq = OSQCreateEx(10);
 
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
