/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-10-12     Meco Man     first version
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
*                                 MUTUAL EXCLUSION SEMAPHORE MANAGEMENT
*
* Filename : os_mutex.c
* Version  : V2.93.00
*********************************************************************************************************
*/

#ifndef  OS_MUTEX_C
#define  OS_MUTEX_C

#ifndef  OS_MASTER_FILE
#include <ucos_ii.h>
#endif


#if OS_MUTEX_EN > 0u
/*
*********************************************************************************************************
*                                  ACCEPT MUTUAL EXCLUSION SEMAPHORE
*
* Description: This  function checks the mutual exclusion semaphore to see if a resource is available.
*              Unlike OSMutexPend(), OSMutexAccept() does not suspend the calling task if the resource is
*              not available or the event did not occur.
*
* Arguments  : pevent     is a pointer to the event control block
*
*              perr       is a pointer to an error code which will be returned to your application:
*                            OS_ERR_NONE         if the call was successful.
*                            OS_ERR_EVENT_TYPE   if 'pevent' is not a pointer to a mutex
*                            OS_ERR_PEVENT_NULL  'pevent' is a NULL pointer
*                            OS_ERR_PEND_ISR     if you called this function from an ISR
*                          - OS_ERR_PCP_LOWER    If the priority of the task that owns the Mutex is
*                                                HIGHER (i.e. a lower number) than the PCP.  This error
*                                                indicates that you did not set the PCP higher (lower
*                                                number) than ALL the tasks that compete for the Mutex.
*                                                Unfortunately, this is something that could not be
*                                                detected when the Mutex is created because we don't know
*                                                what tasks will be using the Mutex.
*
* Returns    : == OS_TRUE    if the resource is available, the mutual exclusion semaphore is acquired
*              == OS_FALSE   a) if the resource is not available
*                            b) you didn't pass a pointer to a mutual exclusion semaphore
*                            c) you called this function from an ISR
*
* Warning(s) : This function CANNOT be called from an ISR because mutual exclusion semaphores are
*              intended to be used by tasks only.
*********************************************************************************************************
*/

#if OS_MUTEX_ACCEPT_EN > 0u
BOOLEAN  OSMutexAccept (OS_EVENT  *pevent,
                        INT8U     *perr)
{
    rt_mutex_t pmutex;

#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (OS_FALSE);
    }
#endif

    pmutex = (rt_mutex_t)pevent->ipc_ptr;
#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                     /* Validate 'pevent'                            */
        *perr = OS_ERR_PEVENT_NULL;
        return (OS_FALSE);
    }
#endif
    if (rt_object_get_type(&pmutex->parent.parent)     /* Validate event block type                */
        != RT_Object_Class_Mutex) {
        *perr = OS_ERR_EVENT_TYPE;
        return (OS_FALSE);
    }
    if (OSIntNesting > 0u) {                           /* Make sure it's not called from an ISR        */
        *perr = OS_ERR_PEND_ISR;
        return (OS_FALSE);
    }

    *perr = OS_ERR_NONE;
    
    if(rt_mutex_take(pmutex, RT_WAITING_NO) == RT_EOK) {
        return (OS_TRUE);
    }
    
    return (OS_FALSE);
}
#endif


/*
*********************************************************************************************************
*                                 CREATE A MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function creates a mutual exclusion semaphore.
*
* Arguments  : prio          is the priority to use when accessing the mutual exclusion semaphore.  In
*                            other words, when the semaphore is acquired and a higher priority task
*                            attempts to obtain the semaphore then the priority of the task owning the
*                            semaphore is raised to this priority.  It is assumed that you will specify
*                            a priority that is LOWER in value than ANY of the tasks competing for the
*                            mutex. If the priority is specified as OS_PRIO_MUTEX_CEIL_DIS, then the
*                            priority ceiling promotion is disabled. This way, the tasks accessing the
*                            semaphore do not have their priority promoted.
*                            由于RT-Thread内核支持同一优先级含多个任务,因此prio在本兼容层无用,随便填什么都行
*
*              perr          is a pointer to an error code which will be returned to your application:
*                               OS_ERR_NONE                     if the call was successful.
*                               OS_ERR_CREATE_ISR               if you attempted to create a MUTEX from an
*                                                               ISR
*                               OS_ERR_ILLEGAL_CREATE_RUN_TIME  if you tried to create a mutex after
*                                                               safety critical operation started.
*                             - OS_ERR_PRIO_EXIST               if a task at the priority ceiling priority
*                                                               already exist.
*                               OS_ERR_PEVENT_NULL              No more event control blocks available.
*                             - OS_ERR_PRIO_INVALID             if the priority you specify is higher that
*                                                               the maximum allowed (i.e. > OS_LOWEST_PRIO)
*
* Returns    : != (void *)0  is a pointer to the event control clock (OS_EVENT) associated with the
*                            created mutex.
*              == (void *)0  if an error is detected.
*
* Note(s)    : 1) The LEAST significant 8 bits of '.OSEventCnt' hold the priority number of the task
*                 owning the mutex or 0xFF if no task owns the mutex.
*
*              2) The MOST  significant 8 bits of '.OSEventCnt' hold the priority number used to
*                 reduce priority inversion or 0xFF (OS_PRIO_MUTEX_CEIL_DIS) if priority ceiling
*                 promotion is disabled.
*********************************************************************************************************
*/

OS_EVENT  *OSMutexCreate (INT8U   prio,
                          INT8U  *perr)
{
    OS_EVENT  *pevent;                                            /* 由于RT-Thread内核支持同一优先级含多个任务,因此prio在本兼容层无用*/

#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_EVENT *)0);
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        *perr = OS_ERR_ILLEGAL_CREATE_RUN_TIME;
        return ((OS_EVENT *)0);
    }
#endif

    if (OSIntNesting > 0u) {                               /* See if called from ISR ...               */
        *perr = OS_ERR_CREATE_ISR;                         /* ... can't CREATE mutex from an ISR       */
        return ((OS_EVENT *)0);
    }

    pevent = RT_KERNEL_MALLOC(sizeof(OS_EVENT));           /* Get next free event control block        */
    if (pevent == (OS_EVENT *)0) {                         /* See if an ECB was available              */
       *perr = OS_ERR_PEVENT_NULL;                         /* No more event control blocks             */
        return (pevent);
    }
    
    pevent->ipc_ptr = (struct rt_ipc_object *)
        rt_mutex_create("uCOS-II", RT_IPC_FLAG_PRIO);
    if(pevent->ipc_ptr == 0) {
        RT_KERNEL_FREE(pevent);
        *perr = OS_ERR_PEVENT_NULL;
        return ((OS_EVENT *)0); 
    }

   *perr = OS_ERR_NONE;
    return (pevent);
}


/*
*********************************************************************************************************
*                                 CREATE A MUTUAL EXCLUSION SEMAPHORE
*   额外实现OSMutexCreateEx()函数，该函数并不在uCOS-II原版的函数中，OSMutexCreate()函数中第一个
* 参数prio在兼容层中没有任何意义，因此该函数将OSMutexCreate()函数中的第一个参数略去，以方便用户
* 使用。原因是由于uCOS-II的实现方式过于落后，不支持相同任务在同一优先级。
*   推荐用户使用这个API
*********************************************************************************************************
*/
OS_EVENT  *OSMutexCreateEx (INT8U  *perr)
{
    return OSMutexCreate(0,perr);
}


/*
*********************************************************************************************************
*                                           DELETE A MUTEX
*
* Description: This function deletes a mutual exclusion semaphore and readies all tasks pending on the it.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mutex.
*
*              opt           determines delete options as follows:
*                            opt == OS_DEL_NO_PEND   Delete mutex ONLY if no task pending
*                            opt == OS_DEL_ALWAYS    Deletes the mutex even if tasks are waiting.
*                                                    In this case, all the tasks pending will be readied.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            OS_ERR_NONE                  The call was successful and the mutex was deleted
*                            OS_ERR_DEL_ISR               If you attempted to delete the MUTEX from an ISR
*                            OS_ERR_INVALID_OPT           An invalid option was specified
*                            OS_ERR_ILLEGAL_DEL_RUN_TIME  If you tried to delete a mutex after safety
*                                                         critical operation started.
*                            OS_ERR_TASK_WAITING          One or more tasks were waiting on the mutex
*                            OS_ERR_EVENT_TYPE            If you didn't pass a pointer to a mutex
*                            OS_ERR_PEVENT_NULL           If 'pevent' is a NULL pointer.
*
* Returns    : pevent        upon error
*              (OS_EVENT *)0 if the mutex was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the mutex MUST check the return code of OSMutexPend().
*
*              2) This call can potentially disable interrupts for a long time.  The interrupt disable
*                 time is directly proportional to the number of tasks waiting on the mutex.
*
*              3) Because ALL tasks pending on the mutex will be readied, you MUST be careful because the
*                 resource(s) will no longer be guarded by the mutex.
*
*              4) IMPORTANT: In the 'OS_DEL_ALWAYS' case, we assume that the owner of the Mutex (if there
*                            is one) is ready-to-run and is thus NOT pending on another kernel object or
*                            has delayed itself.  In other words, if a task owns the mutex being deleted,
*                            that task will be made ready-to-run at its original priority.
*********************************************************************************************************
*/

#if OS_MUTEX_DEL_EN > 0u
OS_EVENT  *OSMutexDel (OS_EVENT  *pevent,
                       INT8U      opt,
                       INT8U     *perr)
{
    OS_EVENT  *pevent_return;
    rt_mutex_t pmutex;

#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_EVENT *)0);
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        *perr = OS_ERR_ILLEGAL_DEL_RUN_TIME;
        return ((OS_EVENT *)0);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                         /* Validate 'pevent'                        */
        *perr = OS_ERR_PEVENT_NULL;
        return (pevent);
    }
#endif

    pmutex = (rt_mutex_t)pevent->ipc_ptr;

    if (rt_object_get_type(&pmutex->parent.parent)         /* Validate event block type                */
        != RT_Object_Class_Mutex) {
        *perr = OS_ERR_EVENT_TYPE;
        return (pevent);
    }
    if (OSIntNesting > 0u) {                               /* See if called from ISR ...               */
        *perr = OS_ERR_DEL_ISR;                            /* ... can't DELETE from an ISR             */
        return (pevent);
    }

    switch (opt) {
        case OS_DEL_NO_PEND:                               /* DELETE MUTEX ONLY IF NO TASK WAITING --- */
            if(rt_list_isempty(&(pmutex->parent.suspend_thread))) { /* 若没有线程等待信号量            */
                rt_mutex_delete(pmutex);                   /* invoke RT-Thread API                     */
                RT_KERNEL_FREE(pevent);
                *perr = OS_ERR_NONE;
                pevent_return =  (OS_EVENT *)0; 
            } else {
                *perr = OS_ERR_TASK_WAITING;
                pevent_return = pevent; 
            }
            break;

        case OS_DEL_ALWAYS:                                /* ALWAYS DELETE THE MUTEX ---------------- */
            rt_mutex_delete(pmutex);                       /* invoke RT-Thread API                     */
            RT_KERNEL_FREE(pevent);
            *perr = OS_ERR_NONE;
            pevent_return =  (OS_EVENT *)0; 
            break;

        default:
            *perr         = OS_ERR_INVALID_OPT;
            pevent_return = pevent;
            break;
    }

    return (pevent_return);
}
#endif


/*
*********************************************************************************************************
*                                 PEND ON MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function waits for a mutual exclusion semaphore.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            mutex.
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will
*                            wait for the resource up to the amount of time specified by this argument.
*                            If you specify 0, however, your task will wait forever at the specified
*                            mutex or, until the resource becomes available.
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*                               OS_ERR_NONE        The call was successful and your task owns the mutex
*                               OS_ERR_TIMEOUT     The mutex was not available within the specified 'timeout'.
*                               OS_ERR_PEND_ABORT  The wait on the mutex was aborted.
*                               OS_ERR_EVENT_TYPE  If you didn't pass a pointer to a mutex
*                               OS_ERR_PEVENT_NULL 'pevent' is a NULL pointer
*                               OS_ERR_PEND_ISR    If you called this function from an ISR and the result
*                                                  would lead to a suspension.
*                             - OS_ERR_PCP_LOWER   If the priority of the task that owns the Mutex is
*                                                  HIGHER (i.e. a lower number) than the PCP.  This error
*                                                  indicates that you did not set the PCP higher (lower
*                                                  number) than ALL the tasks that compete for the Mutex.
*                                                  Unfortunately, this is something that could not be
*                                                  detected when the Mutex is created because we don't know
*                                                  what tasks will be using the Mutex.
*                               OS_ERR_PEND_LOCKED If you called this function when the scheduler is locked
*
* Returns    : none
*
* Note(s)    : 1) The task that owns the Mutex MUST NOT pend on any other event while it owns the mutex.
*
*              2) You MUST NOT change the priority of the task that owns the mutex
*********************************************************************************************************
*/

void  OSMutexPend (OS_EVENT  *pevent,
                   INT32U     timeout,
                   INT8U     *perr)
{
    rt_mutex_t pmutex;
    rt_err_t   rt_err;
#if OS_CRITICAL_METHOD == 3u                               /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0u;
#endif


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                         /* Validate 'pevent'                        */
        *perr = OS_ERR_PEVENT_NULL;
        return;
    }
#endif
    
    pmutex = (rt_mutex_t)pevent->ipc_ptr;
    
    if (rt_object_get_type(&pmutex->parent.parent)         /* Validate event block type                */
        != RT_Object_Class_Mutex) {
        *perr = OS_ERR_EVENT_TYPE;
        return;
    }
    if (OSIntNesting > 0u) {                               /* See if called from ISR ...               */
        *perr = OS_ERR_PEND_ISR;                           /* ... can't PEND from an ISR               */
        return;
    }
    if (OSLockNesting > 0u) {                              /* See if called with scheduler locked ...  */
        *perr = OS_ERR_PEND_LOCKED;                        /* ... can't PEND when locked               */
        return;
    }

    OS_ENTER_CRITICAL();   
    OSTCBCur->OSTCBStat     |= OS_STAT_MUTEX;         /* Mutex not available, pend current task        */
    OSTCBCur->OSTCBStatPend  = OS_STAT_PEND_OK;
    OSTCBCur->OSTCBDly       = timeout;               /* Store timeout in current task's TCB           */
    OSTCBCur->OSTCBEventPtr  = pevent;
    OS_EXIT_CRITICAL();

    if(timeout) {                                     /* 0为永久等待                                   */
        rt_err = rt_mutex_take(pmutex, timeout);
        OS_ENTER_CRITICAL(); 
        if (rt_err == RT_EOK) {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_OK;
        } else if(OSTCBCur->OSTCBStatPend == OS_STAT_PEND_ABORT) {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_ABORT;
        } else {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_TO;
        }
    }else {
        rt_mutex_take(pmutex, RT_WAITING_FOREVER);
        OS_ENTER_CRITICAL(); 
        if(OSTCBCur->OSTCBStatPend == OS_STAT_PEND_ABORT) {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_ABORT;
        }else {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_OK;
        }
    }
    switch (OSTCBCur->OSTCBStatPend) {                /* See if we timed-out or aborted                */
        case OS_STAT_PEND_OK:
             *perr = OS_ERR_NONE;
             break;

        case OS_STAT_PEND_ABORT:
             *perr = OS_ERR_PEND_ABORT;               /* Indicate that we aborted getting mutex        */
             break;

        case OS_STAT_PEND_TO:
        default:
             *perr = OS_ERR_TIMEOUT;                  /* Indicate that we didn't get mutex within TO   */
             break;
    }
    OSTCBCur->OSTCBStat          =  OS_STAT_RDY;      /* Set   task  status to ready                   */
    OSTCBCur->OSTCBStatPend      =  OS_STAT_PEND_OK;  /* Clear pend  status                            */
    OSTCBCur->OSTCBEventPtr      = (OS_EVENT  *)0;    /* Clear event pointers                          */
    OS_EXIT_CRITICAL();
}


/*
*********************************************************************************************************
*                                POST TO A MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function signals a mutual exclusion semaphore
*
* Arguments  : pevent              is a pointer to the event control block associated with the desired
*                                  mutex.
*
* Returns    : OS_ERR_NONE             The call was successful and the mutex was signaled.
*              OS_ERR_EVENT_TYPE       If you didn't pass a pointer to a mutex
*              OS_ERR_PEVENT_NULL      'pevent' is a NULL pointer
*              OS_ERR_POST_ISR         Attempted to post from an ISR (not valid for MUTEXes)
*              OS_ERR_NOT_MUTEX_OWNER  The task that did the post is NOT the owner of the MUTEX.
*            - OS_ERR_PCP_LOWER        If the priority of the new task that owns the Mutex is
*                                      HIGHER (i.e. a lower number) than the PCP.  This error
*                                      indicates that you did not set the PCP higher (lower
*                                      number) than ALL the tasks that compete for the Mutex.
*                                      Unfortunately, this is something that could not be
*                                      detected when the Mutex is created because we don't know
*                                      what tasks will be using the Mutex.
*********************************************************************************************************
*/

INT8U  OSMutexPost (OS_EVENT *pevent)
{
    rt_mutex_t pmutex;
#if OS_CRITICAL_METHOD == 3u                          /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    if (OSIntNesting > 0u) {                          /* See if called from ISR ...                    */
        return (OS_ERR_POST_ISR);                     /* ... can't POST mutex from an ISR              */
    }
#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        return (OS_ERR_PEVENT_NULL);
    }
#endif

    pmutex = (rt_mutex_t)pevent->ipc_ptr;
    
    if (rt_object_get_type(&pmutex->parent.parent)    /* Validate event block type                     */
        != RT_Object_Class_Mutex) {
        return (OS_ERR_EVENT_TYPE);
    }
    OS_ENTER_CRITICAL();
    if (OSTCBCur != (OS_TCB *)pmutex->owner) {        /* See if posting task owns the MUTEX            */
        OS_EXIT_CRITICAL();
        return (OS_ERR_NOT_MUTEX_OWNER);
    }
    OS_EXIT_CRITICAL();
    
    rt_mutex_release(pmutex);                         /* invoke rt-thread API                          */
    return (OS_ERR_NONE);
}


/*
*********************************************************************************************************
*                                 QUERY A MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function obtains information about a mutex
*
* Arguments  : pevent          is a pointer to the event control block associated with the desired mutex
*
*              p_mutex_data    is a pointer to a structure that will contain information about the mutex
*
* Returns    : OS_ERR_NONE          The call was successful and the message was sent
*              OS_ERR_QUERY_ISR     If you called this function from an ISR
*              OS_ERR_PEVENT_NULL   If 'pevent'       is a NULL pointer
*              OS_ERR_PDATA_NULL    If 'p_mutex_data' is a NULL pointer
*              OS_ERR_EVENT_TYPE    If you are attempting to obtain data from a non mutex.
*********************************************************************************************************
*/

#if OS_MUTEX_QUERY_EN > 0u
INT8U  OSMutexQuery (OS_EVENT       *pevent,
                     OS_MUTEX_DATA  *p_mutex_data)
{
    rt_mutex_t pmutex;
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR   cpu_sr = 0u;
#endif

    pmutex = (rt_mutex_t)pevent->ipc_ptr;

    if (OSIntNesting > 0u) {                               /* See if called from ISR ...               */
        return (OS_ERR_QUERY_ISR);                         /* ... can't QUERY mutex from an ISR        */
    }
#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                         /* Validate 'pevent'                        */
        return (OS_ERR_PEVENT_NULL);
    }
    if (p_mutex_data == (OS_MUTEX_DATA *)0) {              /* Validate 'p_mutex_data'                  */
        return (OS_ERR_PDATA_NULL);
    }
#endif
    if (rt_object_get_type(&pmutex->parent.parent)         /* Validate event block type                */
        != RT_Object_Class_Mutex) {
        return (OS_ERR_EVENT_TYPE);
    }

    OS_ENTER_CRITICAL();
    rt_memcpy(&p_mutex_data->OSMutex, pmutex, sizeof(struct rt_mutex));
    p_mutex_data->OSOwnerPrio = pmutex->owner->current_priority;
    OS_EXIT_CRITICAL();
    return (OS_ERR_NONE);
}
#endif                                                     /* OS_MUTEX_QUERY_EN                        */

#endif                                                     /* OS_MUTEX_EN                              */
#endif                                                     /* OS_MUTEX_C                               */
