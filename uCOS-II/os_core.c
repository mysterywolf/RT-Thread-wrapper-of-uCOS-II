/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-18     Meco Man     first version
 */
/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                    Copyright 1992-2020 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                            CORE FUNCTIONS
*
* Filename : os_core.c
* Version  : V2.93.00
*********************************************************************************************************
*/

#define  OS_GLOBALS
#include "ucos_ii.h"

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INITIALIZATION
*                                    INITIALIZE MISCELLANEOUS VARIABLES
*
* Description: This function is called by OSInit() to initialize miscellaneous variables.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

static  void  OS_InitMisc (void)
{
    OSTaskCtr                 = 0u;                        /* Clear the number of tasks                */
    OSRunning                 = OS_TRUE;                   /* 初始化时,rt-thread已经启动因此直接为OS_TRUE*/

#if OS_TASK_STAT_EN > 0u
    OSIdleCtrRun              = 0uL;
    OSIdleCtrMax              = 0uL;
    OSStatRdy                 = OS_FALSE;                  /* Statistic task is not ready              */
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    OSSafetyCriticalStartFlag = OS_FALSE;                  /* Still allow creation of objects          */
#endif

#if OS_TASK_REG_TBL_SIZE > 0u
    OSTaskRegNextAvailID      = 0u;                        /* Initialize the task register ID          */
#endif
}


/*
*********************************************************************************************************
*                                             INITIALIZATION
*                            INITIALIZE THE FREE LIST OF TASK CONTROL BLOCKS
*
* Description: This function is called by OSInit() to initialize the free list of OS_TCBs.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

static  void  OS_InitTCBList (void)
{
    INT8U    ix;
    INT8U    ix_next;
    OS_TCB  *ptcb1;
    OS_TCB  *ptcb2;

    OS_MemClr((INT8U *)&OSTCBPrioTbl[0], sizeof(OSTCBPrioTbl));  /* Clear the priority table           */
    OS_MemClr((INT8U *)&OSTCBTbl[0],     sizeof(OSTCBTbl));      /* Clear all the TCBs                 */
    for (ix = 0u; ix < (OS_MAX_TASKS + OS_N_SYS_TASKS - 1u); ix++) {    /* Init. list of free TCBs     */
        ix_next =  ix + 1u;
        ptcb1   = &OSTCBTbl[ix];
        ptcb2   = &OSTCBTbl[ix_next];
        ptcb1->OSTCBNext = ptcb2;
#if OS_TASK_NAME_EN > 0u
        ptcb1->OSTCBTaskName = (INT8U *)(void *)"?";             /* Unknown name                       */
#endif
    }
    ptcb1                   = &OSTCBTbl[ix];
    ptcb1->OSTCBNext        = (OS_TCB *)0;                       /* Last OS_TCB                        */
#if OS_TASK_NAME_EN > 0u
    ptcb1->OSTCBTaskName    = (INT8U *)(void *)"?";              /* Unknown name                       */
#endif
    OSTCBList               = (OS_TCB *)0;                       /* TCB lists initializations          */
    OSTCBFreeList           = &OSTCBTbl[0];

    OSTCBPrioTbl[OS_MAX_TASKS + OS_N_SYS_TASKS - 1u] = OS_TCB_RESERVED; /* 空闲任务标记为已经使用      */
}


#if OS_TASK_STAT_EN > 0u
/*
*********************************************************************************************************
*                                             INITIALIZATION
*                                         CREATING THE IDLE TASK
*
* Description: This function creates the Idle Task.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

static  void  OS_InitTaskIdle (void)
{
    rt_thread_idle_sethook(OS_TaskIdle);                    /*向RTT注册μCOS-III兼容层空闲任务(实则为回调函数)         */
}
#endif


/*
*********************************************************************************************************
*                                             INITIALIZATION
*                                      CREATING THE STATISTIC TASK
*
* Description: This function creates the Statistic Task.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if OS_TASK_STAT_EN > 0u
static  void  OS_InitTaskStat (void)
{
#if OS_TASK_NAME_EN > 0u
    INT8U  err;
#endif


#if OS_TASK_CREATE_EXT_EN > 0u
    #if OS_STK_GROWTH == 1u
    (void)OSTaskCreateExt(OS_TaskStat,
                          (void *)0,                                   /* No args passed to OS_TaskStat()*/
                          &OSTaskStatStk[OS_TASK_STAT_STK_SIZE - 1u],  /* Set Top-Of-Stack               */
                          OS_TASK_STAT_PRIO,                           /* One higher than the idle task  */
                          OS_TASK_STAT_ID,
                          &OSTaskStatStk[0],                           /* Set Bottom-Of-Stack            */
                          OS_TASK_STAT_STK_SIZE,
                          (void *)0,                                   /* No TCB extension               */
                          OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);  /* Enable stack checking + clear  */
    #else
    (void)OSTaskCreateExt(OS_TaskStat,
                          (void *)0,                                   /* No args passed to OS_TaskStat()*/
                          &OSTaskStatStk[0],                           /* Set Top-Of-Stack               */
                          OS_TASK_STAT_PRIO,                           /* One higher than the idle task  */
                          OS_TASK_STAT_ID,
                          &OSTaskStatStk[OS_TASK_STAT_STK_SIZE - 1u],  /* Set Bottom-Of-Stack            */
                          OS_TASK_STAT_STK_SIZE,
                          (void *)0,                                   /* No TCB extension               */
                          OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);  /* Enable stack checking + clear  */
    #endif
#else
    #if OS_STK_GROWTH == 1u
    (void)OSTaskCreate(OS_TaskStat,
                       (void *)0,                                      /* No args passed to OS_TaskStat()*/
                       &OSTaskStatStk[OS_TASK_STAT_STK_SIZE - 1u],     /* Set Top-Of-Stack               */
                       OS_TASK_STAT_PRIO);                             /* One higher than the idle task  */
    #else
    (void)OSTaskCreate(OS_TaskStat,
                       (void *)0,                                      /* No args passed to OS_TaskStat()*/
                       &OSTaskStatStk[0],                              /* Set Top-Of-Stack               */
                       OS_TASK_STAT_PRIO);                             /* One higher than the idle task  */
    #endif
#endif

#if OS_TASK_NAME_EN > 0u
    OSTaskNameSet(OS_TASK_STAT_PRIO, (INT8U *)(void *)"uC/OS-II Stat", &err);
#endif
}
#endif


/*
*********************************************************************************************************
*                                           INITIALIZATION
*
* Description: This function is used to initialize the internals of uC/OS-II and MUST be called prior to
*              creating any uC/OS-II object and, prior to calling OSStart().
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

void  OSInit (void)
{
#if OS_TASK_CREATE_EXT_EN > 0u
#if defined(OS_TLS_TBL_SIZE) && (OS_TLS_TBL_SIZE > 0u)
    INT8U  err;
#endif
#endif

    OSSchedLock();

#if OS_CPU_HOOKS_EN > 0u
    OSInitHookBegin();                                           /* Call port specific initialization code   */
#endif
    OS_InitMisc();                                               /* Initialize miscellaneous variables       */
    OS_InitTCBList();                                            /* Initialize the free list of OS_TCBs      */

#if (OS_MEM_EN > 0u) && (OS_MAX_MEM_PART > 0u)
    OS_MemInit();                                                /* Initialize the memory manager            */
#endif

#if OS_TASK_STAT_EN > 0u
    OS_InitTaskIdle();                                           /* Create the Idle Task                     */
    OS_InitTaskStat();                                           /* Create the Statistic Task                */
#endif

#if OS_CPU_HOOKS_EN > 0u
    OSInitHookEnd();                                             /* Call port specific init. code            */
#endif

    OSSchedUnlock();
}


/*
*********************************************************************************************************
*                                              ENTER ISR
*
* Description: This function is used to notify uC/OS-II that you are about to service an interrupt
*              service routine (ISR).  This allows uC/OS-II to keep track of interrupt nesting and thus
*              only perform rescheduling at the last nested ISR.
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) This function should be called with interrupts already disabled
*              2) Your ISR can directly increment OSIntNesting without calling this function because
*                 OSIntNesting has been declared 'global'.
*              3) You MUST still call OSIntExit() even though you increment OSIntNesting directly.
*              4) You MUST invoke OSIntEnter() and OSIntExit() in pair.  In other words, for every call
*                 to OSIntEnter() at the beginning of the ISR you MUST have a call to OSIntExit() at the
*                 end of the ISR.
*              5) You are allowed to nest interrupts up to 255 levels deep.
*              6) I removed the OS_ENTER_CRITICAL() and OS_EXIT_CRITICAL() around the increment because
*                 OSIntEnter() is always called with interrupts disabled.
*********************************************************************************************************
*/

void  OSIntEnter (void)
{
    if (OSRunning == OS_TRUE) {
        rt_interrupt_enter();
    }
}


/*
*********************************************************************************************************
*                                              EXIT ISR
*
* Description: This function is used to notify uC/OS-II that you have completed servicing an ISR.  When
*              the last nested ISR has completed, uC/OS-II will call the scheduler to determine whether
*              a new, high-priority task, is ready to run.
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) You MUST invoke OSIntEnter() and OSIntExit() in pair.  In other words, for every call
*                 to OSIntEnter() at the beginning of the ISR you MUST have a call to OSIntExit() at the
*                 end of the ISR.
*              2) Rescheduling is prevented when the scheduler is locked (see OS_SchedLock())
*********************************************************************************************************
*/

void  OSIntExit (void)
{
    if (OSRunning == OS_TRUE) {
        rt_interrupt_leave();
    }
}


/*
*********************************************************************************************************
*                         INDICATE THAT IT'S NO LONGER SAFE TO CREATE OBJECTS
*
* Description: This function is called by the application code to indicate that all initialization has
*              been completed and that kernel objects are no longer allowed to be created.
*
* Arguments  : none
*
* Returns    : none
*
* Note(s)    : 1) You should call this function when you no longer want to allow application code to
*                 create kernel objects.
*              2) You need to define the macro 'OS_SAFETY_CRITICAL_IEC61508'
*********************************************************************************************************
*/

#ifdef OS_SAFETY_CRITICAL_IEC61508
void  OSSafetyCriticalStart (void)
{
    OSSafetyCriticalStartFlag = OS_TRUE;
}

#endif


/*
*********************************************************************************************************
*                                         PREVENT SCHEDULING
*
* Description: This function is used to prevent rescheduling to take place.  This allows your application
*              to prevent context switches until you are ready to permit context switching.
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) You MUST invoke OSSchedLock() and OSSchedUnlock() in pair.  In other words, for every
*                 call to OSSchedLock() you MUST have a call to OSSchedUnlock().
*********************************************************************************************************
*/

#if OS_SCHED_LOCK_EN > 0u
void  OSSchedLock (void)
{
    if (OSRunning == OS_TRUE) {                  /* Make sure multitasking is running                  */
        rt_enter_critical();
    }
}
#endif


/*
*********************************************************************************************************
*                                          ENABLE SCHEDULING
*
* Description: This function is used to re-allow rescheduling.
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) You MUST invoke OSSchedLock() and OSSchedUnlock() in pair.  In other words, for every
*                 call to OSSchedLock() you MUST have a call to OSSchedUnlock().
*********************************************************************************************************
*/

#if OS_SCHED_LOCK_EN > 0u
void  OSSchedUnlock (void)
{
    if (OSRunning == OS_TRUE) {                            /* Make sure multitasking is running        */
        rt_exit_critical();
    }
}
#endif


/*
*********************************************************************************************************
*                                         START MULTITASKING
*
* Description: This function is used to start the multitasking process which lets uC/OS-II manages the
*              task that you have created.  Before you can call OSStart(), you MUST have called OSInit()
*              and you MUST have created at least one task.
*
* Arguments  : none
*
* Returns    : none
*
* Note       : OSStartHighRdy() MUST:
*                 a) Call OSTaskSwHook() then,
*                 b) Set OSRunning to OS_TRUE.
*                 c) Load the context of the task pointed to by OSTCBHighRdy.
*                 d_ Execute the task.
*********************************************************************************************************
*/

void  OSStart (void)
{
    /*do nothing*/
}


/*
*********************************************************************************************************
*                                      STATISTICS INITIALIZATION
*
* Description: This function is called by your application to establish CPU usage by first determining
*              how high a 32-bit counter would count to in 1 second if no other tasks were to execute
*              during that time.  CPU usage is then determined by a low priority task which keeps track
*              of this 32-bit counter every second but this time, with other tasks running.  CPU usage is
*              determined by:
*
*                                             OSIdleCtr
*                 CPU Usage (%) = 100 * (1 - ------------)
*                                            OSIdleCtrMax
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if OS_TASK_STAT_EN > 0u
void  OSStatInit (void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif


    OSTimeDly(2u);                               /* Synchronize with clock tick                        */
    OS_ENTER_CRITICAL();
    OSIdleCtr    = 0uL;                          /* Clear idle counter                                 */
    OS_EXIT_CRITICAL();
    OSTimeDly(OS_TICKS_PER_SEC / 10u);           /* Determine MAX. idle counter value for 1/10 second  */
    OS_ENTER_CRITICAL();
    OSIdleCtrMax = OSIdleCtr;                    /* Store maximum idle counter count in 1/10 second    */
    OSStatRdy    = OS_TRUE;
    OS_EXIT_CRITICAL();
}
#endif


/*
*********************************************************************************************************
*                                         PROCESS SYSTEM TICK
*
* Description: This function is used to signal to uC/OS-II the occurrence of a 'system tick' (also known
*              as a 'clock tick').  This function should be called by the ticker ISR but, can also be
*              called by a high priority task.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

void  OSTimeTick (void)
{
    rt_tick_increase();
}


/*
*********************************************************************************************************
*                                             GET VERSION
*
* Description: This function is used to return the version number of uC/OS-II.  The returned value
*              corresponds to uC/OS-II's version number multiplied by 10000.  In other words, version
*              2.01.00 would be returned as 20100.
*
* Arguments  : none
*
* Returns    : The version number of uC/OS-II multiplied by 10000.
*********************************************************************************************************
*/

INT16U  OSVersion (void)
{
    return (OS_VERSION);
}


/*
*********************************************************************************************************
*                                      CLEAR A SECTION OF MEMORY
*
* Description: This function is called by other uC/OS-II services to clear a contiguous block of RAM.
*
* Arguments  : pdest    is the start of the RAM to clear (i.e. write 0x00 to)
*
*              size     is the number of bytes to clear.
*
* Returns    : none
*
* Notes      : 1) This function is INTERNAL to uC/OS-II and your application should not call it.
*              2) Note that we can only clear up to 64K bytes of RAM.  This is not an issue because none
*                 of the uses of this function gets close to this limit.
*              3) The clear is done one byte at a time since this will work on any processor irrespective
*                 of the alignment of the destination.
*********************************************************************************************************
*/

void  OS_MemClr (INT8U  *pdest,
                 INT16U  size)
{
    while (size > 0u) {
        *pdest++ = (INT8U)0;
        size--;
    }
}


/*
*********************************************************************************************************
*                                       COPY A BLOCK OF MEMORY
*
* Description: This function is called by other uC/OS-II services to copy a block of memory from one
*              location to another.
*
* Arguments  : pdest    is a pointer to the 'destination' memory block
*
*              psrc     is a pointer to the 'source'      memory block
*
*              size     is the number of bytes to copy.
*
* Returns    : none
*
* Notes      : 1) This function is INTERNAL to uC/OS-II and your application should not call it.  There is
*                 no provision to handle overlapping memory copy.  However, that's not a problem since this
*                 is not a situation that will happen.
*              2) Note that we can only copy up to 64K bytes of RAM
*              3) The copy is done one byte at a time since this will work on any processor irrespective
*                 of the alignment of the source and destination.
*********************************************************************************************************
*/

void  OS_MemCopy (INT8U  *pdest,
                  INT8U  *psrc,
                  INT16U  size)
{
    rt_memcpy(pdest,psrc,size);
}


/*
*********************************************************************************************************
*                               DETERMINE THE LENGTH OF AN ASCII STRING
*
* Description: This function is called by other uC/OS-II services to determine the size of an ASCII string
*              (excluding the NUL character).
*
* Arguments  : psrc     is a pointer to the string for which we need to know the size.
*
* Returns    : The size of the string (excluding the NUL terminating character)
*
* Notes      : 1) This function is INTERNAL to uC/OS-II and your application should not call it.
*              2) The string to check must be less than 255 characters long.
*********************************************************************************************************
*/

#if (OS_EVENT_NAME_EN > 0u) || (OS_FLAG_NAME_EN > 0u) || (OS_MEM_NAME_EN > 0u) || (OS_TASK_NAME_EN > 0u) || (OS_TMR_CFG_NAME_EN > 0u)
INT8U  OS_StrLen (INT8U *psrc)
{
    return rt_strlen((const char*)psrc);
}
#endif


/*
*********************************************************************************************************
*                                              SCHEDULER
*
* Description: This function is called by other uC/OS-II services to determine whether a new, high
*              priority task has been made ready to run.  This function is invoked by TASK level code
*              and is not used to reschedule tasks from ISRs (see OSIntExit() for ISR rescheduling).
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) This function is INTERNAL to uC/OS-II and your application should not call it.
*              2) Rescheduling is prevented when the scheduler is locked (see OS_SchedLock())
*********************************************************************************************************
*/

void  OS_Sched (void)
{
    rt_schedule();
}


#if OS_TASK_STAT_EN > 0u
/*
*********************************************************************************************************
*                                              IDLE TASK
*
* Description: This task is internal to uC/OS-II and executes whenever no other higher priority tasks
*              executes because they are ALL waiting for event(s) to occur.
*
* Arguments  : none
*
* Returns    : none
*
* Note(s)    : 1) OSTaskIdleHook() is called after the critical section to ensure that interrupts will be
*                 enabled for at least a few instructions.  On some processors (ex. Philips XA), enabling
*                 and then disabling interrupts didn't allow the processor enough time to have interrupts
*                 enabled before they were disabled again.  uC/OS-II would thus never recognize
*                 interrupts.
*              2) This hook has been added to allow you to do such things as STOP the CPU to conserve
*                 power.
*              3) 在μCOS-II兼容层中，OS_TaskIdle不再是一个函数，而是一个RT-Thread操作系统Idle任务的回调函数
*********************************************************************************************************
*/

void  OS_TaskIdle (void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL();
    OSIdleCtr++;
    OS_EXIT_CRITICAL();
#if OS_CPU_HOOKS_EN > 0u
    OSTaskIdleHook();                            /* Call user definable HOOK                           */
#endif
}


/*
*********************************************************************************************************
*                                           STATISTICS TASK
*
* Description: This task is internal to uC/OS-II and is used to compute some statistics about the
*              multitasking environment.  Specifically, OS_TaskStat() computes the CPU usage.
*              CPU usage is determined by:
*
*                                          OSIdleCtr
*                 OSCPUUsage = 100 * (1 - ------------)     (units are in %)
*                                         OSIdleCtrMax
*
* Arguments  : parg     this pointer is not used at this time.
*
* Returns    : none
*
* Notes      : 1) This task runs at a priority level higher than the idle task.  In fact, it runs at the
*                 next higher priority, OS_TASK_IDLE_PRIO-1.
*              2) You can disable this task by setting the configuration #define OS_TASK_STAT_EN to 0.
*              3) You MUST have at least a delay of 2/10 seconds to allow for the system to establish the
*                 maximum value for the idle counter.
*********************************************************************************************************
*/

void  OS_TaskStat (void *p_arg)
{
    INT8S  usage;
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif



    p_arg = p_arg;                               /* Prevent compiler warning for not using 'p_arg'     */
    while (OSStatRdy == OS_FALSE) {
        OSTimeDly(2u * OS_TICKS_PER_SEC / 10u);  /* Wait until statistic task is ready                 */
    }
    OSIdleCtrMax /= 100uL;
    if (OSIdleCtrMax == 0uL) {
        OSCPUUsage = 0u;
#if OS_TASK_SUSPEND_EN > 0u
        (void)OSTaskSuspend(OS_PRIO_SELF);
#else
        for (;;) {
            OSTimeDly(OS_TICKS_PER_SEC);
        }
#endif
    }
    OS_ENTER_CRITICAL();
    OSIdleCtr = OSIdleCtrMax * 100uL;            /* Set initial CPU usage as 0%                        */
    OS_EXIT_CRITICAL();
    for (;;) {
        OSTimeDly(1);                            /* Synchronize with clock tick                        */

        OS_ENTER_CRITICAL();
        OSIdleCtr = 0uL;                        /* Reset the idle counter for the next second         */
        OS_EXIT_CRITICAL();

        OSTimeDly(OS_TICKS_PER_SEC / 10u);       /* Accumulate OSIdleCtr for the next 1/10 second      */

        OS_ENTER_CRITICAL();
        OSIdleCtrRun = OSIdleCtr;                /* Store number of cycles which elapsed while idle    */
        OS_EXIT_CRITICAL();

        usage            = 100 - (INT8S)(OSIdleCtrRun / OSIdleCtrMax);
        if (usage >= 0) {                        /* Make sure we don't have a negative percentage      */
            OSCPUUsage   = (INT8U)usage;
        } else {
            OSCPUUsage   = 0u;
            OSIdleCtrMax = OSIdleCtrRun / 100uL; /* Update max counter value to current one            */
        }

#if OS_CPU_HOOKS_EN > 0u
        OSTaskStatHook();                        /* Invoke user definable hook                         */
#endif
#if (OS_TASK_STAT_STK_CHK_EN > 0u) && (OS_TASK_CREATE_EXT_EN > 0u)
        OS_TaskStatStkChk();                     /* Check the stacks for each task                     */
#endif
    }
}
#endif


/*
*********************************************************************************************************
*                                        CHECK ALL TASK STACKS
*
* Description: This function is called by OS_TaskStat() to check the stacks of each active task.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if (OS_TASK_STAT_STK_CHK_EN > 0u) && (OS_TASK_CREATE_EXT_EN > 0u)
void  OS_TaskStatStkChk (void)
{
    OS_TCB      *ptcb;
    OS_STK_DATA  stk_data;
    INT8U        err;
    INT8U        prio;


    for (prio = 0u; prio <= OS_TASK_IDLE_PRIO; prio++) {
        err = OSTaskStkChk(prio, &stk_data);
        if (err == OS_ERR_NONE) {
            ptcb = OSTCBPrioTbl[prio];
            if (ptcb != (OS_TCB *)0) {                               /* Make sure task 'ptcb' is ...   */
                if (ptcb != OS_TCB_RESERVED) {                       /* ... still valid.               */
#if OS_TASK_PROFILE_EN > 0u
                    #if OS_STK_GROWTH == 1u
                    ptcb->OSTCBStkBase = ptcb->OSTCBStkBottom + ptcb->OSTCBStkSize;
                    #else
                    ptcb->OSTCBStkBase = ptcb->OSTCBStkBottom - ptcb->OSTCBStkSize;
                    #endif
                    ptcb->OSTCBStkUsed = stk_data.OSUsed;            /* Store number of entries used   */
#endif
                }
            }
        }
    }
}
#endif


/*
*********************************************************************************************************
*                                           INITIALIZE TCB
*
* Description: This function is internal to uC/OS-II and is used to initialize a Task Control Block when
*              a task is created (see OSTaskCreate() and OSTaskCreateExt()).
*
* Arguments  : ptcb          兼容层新增参数
*
*              prio          is the priority of the task being created
*
*              ptos          is a pointer to the task's top-of-stack assuming that the CPU registers
*                            have been placed on the stack.  Note that the top-of-stack corresponds to a
*                            'high' memory location is OS_STK_GROWTH is set to 1 and a 'low' memory
*                            location if OS_STK_GROWTH is set to 0.  Note that stack growth is CPU
*                            specific.
*
*              pbos          is a pointer to the bottom of stack.  A NULL pointer is passed if called by
*                            'OSTaskCreate()'.
*
*              id            is the task's ID (0..65535)
*
*              stk_size      is the size of the stack (in 'stack units').  If the stack units are INT8Us
*                            then, 'stk_size' contains the number of bytes for the stack.  If the stack
*                            units are INT32Us then, the stack contains '4 * stk_size' bytes.  The stack
*                            units are established by the #define constant OS_STK which is CPU
*                            specific.  'stk_size' is 0 if called by 'OSTaskCreate()'.
*
*              pext          is a pointer to a user supplied memory area that is used to extend the task
*                            control block.  This allows you to store the contents of floating-point
*                            registers, MMU registers or anything else you could find useful during a
*                            context switch.  You can even assign a name to each task and store this name
*                            in this TCB extension.  A NULL pointer is passed if called by OSTaskCreate().
*
*              opt           options as passed to 'OSTaskCreateExt()' or,
*                            0 if called from 'OSTaskCreate()'.
*
*              pptcb         返回ptcb指针
*
* Returns    : OS_ERR_NONE              if the call was successful
*              OS_ERR_TASK_NO_MORE_TCB  if there are no more free TCBs to be allocated and thus, the task
*                                       cannot be created.
*
* Note       : This function is INTERNAL to uC/OS-II and your application should not call it.
*********************************************************************************************************
*/
INT8U  OS_TCBInit (INT8U    prio,
                   OS_STK  *ptos,
                   OS_STK  *pbos,
                   INT16U   id,
                   INT32U   stk_size,
                   void    *pext,
                   INT16U   opt,
                   OS_TCB **pptcb)
{
    OS_TCB    *ptcb;
#if OS_CRITICAL_METHOD == 3u                               /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0u;
#endif
#if OS_TASK_REG_TBL_SIZE > 0u
    INT8U      i;
#endif
#if OS_TASK_CREATE_EXT_EN > 0u
#if defined(OS_TLS_TBL_SIZE) && (OS_TLS_TBL_SIZE > 0u)
    INT8U      j;
#endif
#endif


    OS_ENTER_CRITICAL();
    ptcb = OSTCBFreeList;                                  /* Get a free TCB from the free TCB list    */
    if (ptcb != (OS_TCB *)0) {
        OSTCBFreeList            = ptcb->OSTCBNext;        /* Update pointer to free TCB list          */
        OS_EXIT_CRITICAL();
        ptcb->OSTCBStkPtr        = ptos;                   /* Load Stack pointer in TCB                */
        ptcb->OSTCBPrio          = prio;                   /* Load task priority into TCB              */
        ptcb->OSTCBStat          = OS_STAT_RDY;            /* Task is ready to run                     */
        ptcb->OSTCBStatPend      = OS_STAT_PEND_OK;        /* Clear pend status                        */
#ifndef PKG_USING_UCOSII_WRAPPER_TINY
        ptcb->OSTCBDly           = 0u;                     /* Task is not delayed                      */
#endif

#if OS_TASK_CREATE_EXT_EN > 0u
        ptcb->OSTCBExtPtr        = pext;                   /* Store pointer to TCB extension           */
        ptcb->OSTCBStkSize       = stk_size;               /* Store stack size                         */
        ptcb->OSTCBStkBottom     = pbos;                   /* Store pointer to bottom of stack         */
        ptcb->OSTCBOpt           = opt;                    /* Store task options                       */
        ptcb->OSTCBId            = id;                     /* Store task ID                            */
#else
        pext                     = pext;                   /* Prevent compiler warning if not used     */
        stk_size                 = stk_size;
        pbos                     = pbos;
        opt                      = opt;
        id                       = id;
#endif

#if OS_TASK_DEL_EN > 0u
        ptcb->OSTCBDelReq        = OS_ERR_NONE;
#endif

#ifndef PKG_USING_UCOSII_WRAPPER_TINY
#if (OS_EVENT_EN)
        ptcb->OSTCBEventPtr      = (OS_EVENT  *)0;         /* Task is not pending on an  event         */
#endif
#endif

#if OS_TASK_PROFILE_EN > 0u
        ptcb->OSTCBStkBase       = (OS_STK *)0;
        ptcb->OSTCBStkUsed       = 0uL;
#endif

#if OS_TASK_NAME_EN > 0u
        ptcb->OSTCBTaskName      = (INT8U *)(void *)"?";
#endif

#if OS_TASK_REG_TBL_SIZE > 0u                              /* Initialize the task variables            */
        for (i = 0u; i < OS_TASK_REG_TBL_SIZE; i++) {
            ptcb->OSTCBRegTbl[i] = 0u;
        }
#endif
#if OS_CPU_HOOKS_EN > 0u
        OSTCBInitHook(ptcb);
#endif
        OS_ENTER_CRITICAL();
        OSTCBPrioTbl[prio] = ptcb;
        OS_EXIT_CRITICAL();
#if OS_CPU_HOOKS_EN > 0u
        OSTaskCreateHook(ptcb);                            /* Call user defined hook                   */
#endif
#if OS_TASK_CREATE_EXT_EN > 0u
#if defined(OS_TLS_TBL_SIZE) && (OS_TLS_TBL_SIZE > 0u)
        for (j = 0u; j < OS_TLS_TBL_SIZE; j++) {
            ptcb->OSTCBTLSTbl[j] = (OS_TLS)0;
        }
        OS_TLS_TaskCreate(ptcb);                           /* Call TLS hook                            */
#endif
#endif

        OS_ENTER_CRITICAL();
        ptcb->OSTCBNext = OSTCBList;                       /* Link into TCB chain                      */
        ptcb->OSTCBPrev = (OS_TCB *)0;
        if (OSTCBList != (OS_TCB *)0) {
            OSTCBList->OSTCBPrev = ptcb;
        }
        OSTCBList               = ptcb;
        OSTaskCtr++;                                       /* Increment the #tasks counter             */
        *pptcb = ptcb;
        OS_EXIT_CRITICAL();
        return (OS_ERR_NONE);
    }
    OS_EXIT_CRITICAL();
    return (OS_ERR_TASK_NO_MORE_TCB);
}
