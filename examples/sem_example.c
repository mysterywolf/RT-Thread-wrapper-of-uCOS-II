#include <ucos_ii.h>

#define TASK_SIZE  256
#define TASK1_PRIO   5
#define TASK2_PRIO   6

static OS_STK task1_stack [TASK_SIZE];
static OS_STK task2_stack [TASK_SIZE];

static OS_EVENT *pSem;

//task1 tx
static void task1(void *p_arg)
{
//    INT8U err; 
    
//    OSTimeDly(1000);
//    OSSemPendAbort(pSem, OS_PEND_OPT_BROADCAST, &err);
    
    while(1)
    {
        OSSemPost(pSem);  
        OSTimeDly(1000);
    }
}

//task2 rx
static void task2(void *p_arg)
{
    INT8U err;
    
    while(1)
    {
        OSSemPend(pSem, 0, &err);
        if(err == OS_ERR_NONE)
        {
            rt_kprintf("success!\r\n");
        }
        else if (err == OS_ERR_PEND_ABORT)
        {
            rt_kprintf("abort!\r\n");
        }
        else
        {
            rt_kprintf("failure!%d\r\n",err);
        }
    }
}

void sem_example (void)
{
    pSem = OSSemCreate(0);
    
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
