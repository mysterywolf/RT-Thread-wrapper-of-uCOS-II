/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-10-06     Meco Man     first version
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
*                                         SEMAPHORE MANAGEMENT
*
* Filename : os_sem.c
* Version  : V2.93.00
*********************************************************************************************************
*/

#include "ucos_ii.h"

#if OS_SEM_EN > 0u
/*
*********************************************************************************************************
*                                          ACCEPT SEMAPHORE
*
* Description: This function checks the semaphore to see if a resource is available or, if an event
*              occurred.  Unlike OSSemPend(), OSSemAccept() does not suspend the calling task if the
*              resource is not available or the event did not occur.
*
* Arguments  : pevent     is a pointer to the event control block
*
* Returns    : >  0       if the resource is available or the event did not occur the semaphore is
*                         decremented to obtain the resource.
*              == 0       if the resource is not available or the event did not occur or,
*                         if 'pevent' is a NULL pointer or,
*                         if you didn't pass a pointer to a semaphore
*********************************************************************************************************
*/

#if OS_SEM_ACCEPT_EN > 0u
INT16U  OSSemAccept (OS_EVENT *pevent)
{
    INT16U     cnt;
    rt_sem_t   psem;

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        return (0u);
    }
#endif
    
    psem = (rt_sem_t)pevent->ipc_ptr;
    
    if (rt_object_get_type(&psem->parent.parent) 
        != RT_Object_Class_Semaphore) {               /* Validate event block type                     */
        return (0u);
    }

    if(rt_sem_take(psem, RT_WAITING_NO) == RT_EOK) {
        cnt = psem->value;
    } else {
        cnt = 0u;
    }
    return (cnt);                                     /* Return semaphore count                        */
}
#endif


/*
*********************************************************************************************************
*                                         CREATE A SEMAPHORE
*
* Description: This function creates a semaphore.
*
* Arguments  : cnt           is the initial value for the semaphore.  If the value is 0, no resource is
*                            available (or no event has occurred).  You initialize the semaphore to a
*                            non-zero value to specify how many resources are available (e.g. if you have
*                            10 resources, you would initialize the semaphore to 10).
*
* Returns    : != (void *)0  is a pointer to the event control block (OS_EVENT) associated with the
*                            created semaphore
*              == (void *)0  if no event control blocks were available
*********************************************************************************************************
*/

OS_EVENT  *OSSemCreate (INT16U cnt)
{
    OS_EVENT  *pevent;
    
#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_EVENT *)0);
    }
#endif

    if (OSIntNesting > 0u) {                               /* See if called from ISR ...               */
        return ((OS_EVENT *)0);                            /* ... can't CREATE from an ISR             */
    }

    pevent = RT_KERNEL_MALLOC(sizeof(OS_EVENT));    
    if (pevent == (OS_EVENT *)0) {                         /* Get an event control block               */
        return ((OS_EVENT *)0);
    }

    pevent->ipc_ptr = (struct rt_ipc_object *)
        rt_sem_create("uCOS-II", cnt, RT_IPC_FLAG_PRIO);
    if(!pevent->ipc_ptr) {
        RT_KERNEL_FREE(pevent);
        return ((OS_EVENT *)0);
    }
 
    return (pevent);
}


/*
*********************************************************************************************************
*                                         DELETE A SEMAPHORE
*
* Description: This function deletes a semaphore and readies all tasks pending on the semaphore.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            semaphore.
*
*              opt           determines delete options as follows:
*                            opt == OS_DEL_NO_PEND   Delete semaphore ONLY if no task pending
*                            opt == OS_DEL_ALWAYS    Deletes the semaphore even if tasks are waiting.
*                                                    In this case, all the tasks pending will be readied.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            OS_ERR_NONE                  The call was successful and the semaphore was
*                                                         deleted
*                            OS_ERR_DEL_ISR               If you attempted to delete the semaphore from an
*                                                         ISR
*                            OS_ERR_ILLEGAL_DEL_RUN_TIME  If you tried to delete the semaphore after
*                                                         safety critical operation started.
*                            OS_ERR_INVALID_OPT           An invalid option was specified
*                            OS_ERR_TASK_WAITING          One or more tasks were waiting on the semaphore
*                            OS_ERR_EVENT_TYPE            If you didn't pass a pointer to a semaphore
*                            OS_ERR_PEVENT_NULL           If 'pevent' is a NULL pointer.
*
* Returns    : pevent        upon error
*              (OS_EVENT *)0 if the semaphore was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the semaphore MUST check the return code of OSSemPend().
*              2) OSSemAccept() callers will not know that the intended semaphore has been deleted unless
*                 they check 'pevent' to see that it's a NULL pointer.
*              3) This call can potentially disable interrupts for a long time.  The interrupt disable
*                 time is directly proportional to the number of tasks waiting on the semaphore.
*              4) Because ALL tasks pending on the semaphore will be readied, you MUST be careful in
*                 applications where the semaphore is used for mutual exclusion because the resource(s)
*                 will no longer be guarded by the semaphore.
*              5) All tasks that were waiting for the semaphore will be readied and returned an
*                 OS_ERR_PEND_ABORT if OSSemDel() was called with OS_DEL_ALWAYS
*********************************************************************************************************
*/

#if OS_SEM_DEL_EN > 0u
OS_EVENT  *OSSemDel (OS_EVENT  *pevent,
                     INT8U      opt,
                     INT8U     *perr)
{
    rt_sem_t   psem;
    OS_EVENT  *pevent_return;

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

    psem = (rt_sem_t)pevent->ipc_ptr;
    
    if (rt_object_get_type(&psem->parent.parent)           /* Validate event block type                */
        != RT_Object_Class_Semaphore) {
        return (pevent);       
    }
    if (OSIntNesting > 0u) {                               /* See if called from ISR ...               */
        *perr = OS_ERR_DEL_ISR;                            /* ... can't DELETE from an ISR             */
        return (pevent);
    }

    switch (opt) {
        case OS_DEL_NO_PEND:                               /* Delete semaphore only if no task waiting */
            if(rt_list_isempty(&(psem->parent.suspend_thread))) { /* 若没有线程等待信号量              */
                rt_sem_delete(psem);                       /* invoke RT-Thread API                     */
                RT_KERNEL_FREE(pevent);
                *perr = OS_ERR_NONE;
                pevent_return =  (OS_EVENT *)0; 
            } else {
                *perr = OS_ERR_TASK_WAITING;
                pevent_return = pevent; 
            }
            break;
            
        case OS_DEL_ALWAYS:                                /* Always delete the semaphore              */
            rt_sem_delete(psem);                           /* invoke RT-Thread API                     */
            RT_KERNEL_FREE(pevent);
            *perr = OS_ERR_NONE;
            pevent_return =  (OS_EVENT *)0; 
            break;

        default:
             *perr                  = OS_ERR_INVALID_OPT;
             pevent_return          = pevent;
             break;
    }

    return (pevent_return);
}
#endif


/*
*********************************************************************************************************
*                                          PEND ON SEMAPHORE
*
* Description: This function waits for a semaphore.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            semaphore.
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will
*                            wait for the resource up to the amount of time specified by this argument.
*                            If you specify 0, however, your task will wait forever at the specified
*                            semaphore or, until the resource becomes available (or the event occurs).
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            OS_ERR_NONE         The call was successful and your task owns the resource
*                                                or, the event you are waiting for occurred.
*                            OS_ERR_TIMEOUT      The semaphore was not received within the specified
*                                                'timeout'.
*                            OS_ERR_PEND_ABORT   The wait on the semaphore was aborted.
*                            OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a semaphore.
*                            OS_ERR_PEND_ISR     If you called this function from an ISR and the result
*                                                would lead to a suspension.
*                            OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
*                            OS_ERR_PEND_LOCKED  If you called this function when the scheduler is locked
*
* Returns    : none
*********************************************************************************************************
*/

void  OSSemPend (OS_EVENT  *pevent,
                 INT32U     timeout,
                 INT8U     *perr)
{
    rt_sem_t   psem;
    rt_err_t   rt_err;
#if OS_CRITICAL_METHOD == 3u                          /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0u;
#endif


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        *perr = OS_ERR_PEVENT_NULL;
        return;
    }
#endif

    psem = (rt_sem_t)pevent->ipc_ptr;

    if (rt_object_get_type(&psem->parent.parent)      /* Validate event block type                */
        != RT_Object_Class_Semaphore) {
        *perr = OS_ERR_EVENT_TYPE;
        return;
    }
    if (OSIntNesting > 0u) {                          /* See if called from ISR ...                    */
        *perr = OS_ERR_PEND_ISR;                      /* ... can't PEND from an ISR                    */
        return;
    }
    if (OSLockNesting > 0u) {                         /* See if called with scheduler locked ...       */
        *perr = OS_ERR_PEND_LOCKED;                   /* ... can't PEND when locked                    */
        return;
    }

    OS_ENTER_CRITICAL();                              /* Otherwise, must wait until event occurs       */
    OSTCBCur->OSTCBStat     |= OS_STAT_SEM;           /* Resource not available, pend on semaphore     */
    OSTCBCur->OSTCBStatPend  = OS_STAT_PEND_OK;
    OSTCBCur->OSTCBDly       = timeout;               /* Store pend timeout in TCB                     */
    OSTCBCur->OSTCBEventPtr  = pevent;
    OS_EXIT_CRITICAL();

    if(timeout) {                                     /* 0为永久等待                                   */
        rt_err = rt_sem_take(psem, timeout);
        OS_ENTER_CRITICAL();
        if (rt_err == RT_EOK) {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_OK;
        } else if(OSTCBCur->OSTCBStatPend == OS_STAT_PEND_ABORT) {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_ABORT;
        } else {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_TO;
        }
    }else {
        rt_sem_take(psem, RT_WAITING_FOREVER);
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
             *perr = OS_ERR_PEND_ABORT;               /* Indicate that we aborted                      */
             break;

        case OS_STAT_PEND_TO:
        default:
             *perr = OS_ERR_TIMEOUT;                  /* Indicate that we didn't get event within TO   */
             break;
    }
    OSTCBCur->OSTCBStat          =  OS_STAT_RDY;      /* Set   task  status to ready                   */
    OSTCBCur->OSTCBStatPend      =  OS_STAT_PEND_OK;  /* Clear pend  status                            */
    OSTCBCur->OSTCBEventPtr      = (OS_EVENT  *)0;    /* Clear event pointers                          */
    OS_EXIT_CRITICAL();
}


/*
*********************************************************************************************************
*                                    ABORT WAITING ON A SEMAPHORE
*
* Description: This function aborts & readies any tasks currently waiting on a semaphore.  This function
*              should be used to fault-abort the wait on the semaphore, rather than to normally signal
*              the semaphore via OSSemPost().
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            semaphore.
*
*              opt           determines the type of ABORT performed:
*                            OS_PEND_OPT_NONE         ABORT wait for a single task (HPT) waiting on the
*                                                     semaphore
*                            OS_PEND_OPT_BROADCAST    ABORT wait for ALL tasks that are  waiting on the
*                                                     semaphore
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            OS_ERR_NONE         No tasks were     waiting on the semaphore.
*                            OS_ERR_PEND_ABORT   At least one task waiting on the semaphore was readied
*                                                and informed of the aborted wait; check return value
*                                                for the number of tasks whose wait on the semaphore
*                                                was aborted.
*                            OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a semaphore.
*                            OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
*
* Returns    : == 0          if no tasks were waiting on the semaphore, or upon error.
*              >  0          if one or more tasks waiting on the semaphore are now readied and informed.
*********************************************************************************************************
*/

#if OS_SEM_PEND_ABORT_EN > 0u
INT8U  OSSemPendAbort (OS_EVENT  *pevent,
                       INT8U      opt,
                       INT8U     *perr)
{
    rt_sem_t   psem;
    INT8U      nbr_tasks = 0u;
    
#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        *perr = OS_ERR_PEVENT_NULL;
        return (0u);
    }
#endif

    psem = (rt_sem_t)pevent->ipc_ptr;

    if (rt_object_get_type(&psem->parent.parent)      /* Validate event block type                */
        != RT_Object_Class_Semaphore) {
        *perr = OS_ERR_EVENT_TYPE;
        return (0u);
    }

    switch (opt)
    {
        case OS_ERR_PEND_ABORT:
            nbr_tasks = rt_ipc_pend_abort_all(&(psem->parent.suspend_thread));

        case OS_PEND_OPT_NONE:
        default:
            rt_ipc_pend_abort_1(&(psem->parent.suspend_thread));
            nbr_tasks = 1u;
    }
    *perr = OS_ERR_NONE;
    return (nbr_tasks);                               /* No tasks waiting on semaphore                 */
}
#endif


/*
*********************************************************************************************************
*                                         POST TO A SEMAPHORE
*
* Description: This function signals a semaphore
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            semaphore.
*
* Returns    : OS_ERR_NONE         The call was successful and the semaphore was signaled.
*              OS_ERR_SEM_OVF      If the semaphore count exceeded its limit. In other words, you have
*                                  signaled the semaphore more often than you waited on it with either
*                                  OSSemAccept() or OSSemPend().
*              OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a semaphore
*              OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
*********************************************************************************************************
*/

INT8U  OSSemPost (OS_EVENT *pevent)
{
    rt_sem_t   psem;

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        return (OS_ERR_PEVENT_NULL);
    }
#endif

    psem = (rt_sem_t)pevent->ipc_ptr;
    
    if (rt_object_get_type(&psem->parent.parent)      /* Validate event block type                     */
        != RT_Object_Class_Semaphore) {
        return (OS_ERR_EVENT_TYPE);
    }
    if (rt_sem_release(psem) == RT_EOK) {
        return (OS_ERR_NONE);
    }   
    return (OS_ERR_SEM_OVF);
}


/*
*********************************************************************************************************
*                                          QUERY A SEMAPHORE
*
* Description: This function obtains information about a semaphore
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            semaphore
*
*              p_sem_data    is a pointer to a structure that will contain information about the
*                            semaphore.
*
* Returns    : OS_ERR_NONE         The call was successful and the message was sent
*              OS_ERR_EVENT_TYPE   If you are attempting to obtain data from a non semaphore.
*              OS_ERR_PEVENT_NULL  If 'pevent'     is a NULL pointer.
*              OS_ERR_PDATA_NULL   If 'p_sem_data' is a NULL pointer
*********************************************************************************************************
*/

#if OS_SEM_QUERY_EN > 0u
INT8U  OSSemQuery (OS_EVENT     *pevent,
                   OS_SEM_DATA  *p_sem_data)
{
    rt_sem_t    psem;
#if OS_CRITICAL_METHOD == 3u                               /* Allocate storage for CPU status register */
    OS_CPU_SR   cpu_sr = 0u;
#endif

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                         /* Validate 'pevent'                        */
        return (OS_ERR_PEVENT_NULL);
    }
    if (p_sem_data == (OS_SEM_DATA *)0) {                  /* Validate 'p_sem_data'                    */
        return (OS_ERR_PDATA_NULL);
    }
#endif
    
    psem = (rt_sem_t)(pevent->ipc_ptr);
    
    if (rt_object_get_type(&psem->parent.parent)           /* Validate event block type                */
        != RT_Object_Class_Semaphore) {
        return (OS_ERR_EVENT_TYPE);
    }

    OS_ENTER_CRITICAL();
    rt_memcpy(&p_sem_data->OSSem, psem, sizeof(struct rt_semaphore));
    p_sem_data->OSCnt = psem->value;
    OS_EXIT_CRITICAL();
    return (OS_ERR_NONE);
}
#endif                                                     /* OS_SEM_QUERY_EN                          */


/*
*********************************************************************************************************
*                                            SET SEMAPHORE
*
* Description: This function sets the semaphore count to the value specified as an argument.  Typically,
*              this value would be 0.
*
*              You would typically use this function when a semaphore is used as a signaling mechanism
*              and, you want to reset the count value.
*
* Arguments  : pevent     is a pointer to the event control block
*
*              cnt        is the new value for the semaphore count.  You would pass 0 to reset the
*                         semaphore count.
*
*              perr       is a pointer to an error code returned by the function as follows:
*
*                            OS_ERR_NONE          The call was successful and the semaphore value was set.
*                            OS_ERR_EVENT_TYPE    If you didn't pass a pointer to a semaphore.
*                            OS_ERR_PEVENT_NULL   If 'pevent' is a NULL pointer.
*                            OS_ERR_TASK_WAITING  If tasks are waiting on the semaphore.
*********************************************************************************************************
*/

#if OS_SEM_SET_EN > 0u
void  OSSemSet (OS_EVENT  *pevent,
                INT16U     cnt,
                INT8U     *perr)
{
    rt_sem_t psem;
#if OS_CRITICAL_METHOD == 3u                          /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0u;
#endif

#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif
#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        *perr = OS_ERR_PEVENT_NULL;
        return;
    }
#endif
    
    psem = (rt_sem_t)pevent->ipc_ptr;
    
    if (rt_object_get_type(&psem->parent.parent)      /* Validate event block type                     */
        != RT_Object_Class_Semaphore) {
        *perr = OS_ERR_EVENT_TYPE;
        return;
    }

    OS_ENTER_CRITICAL();
    *perr = OS_ERR_NONE;
    if (psem->value>0) {
        psem->value = cnt;
    } else {
        if(rt_list_isempty(&(psem->parent.suspend_thread))) {  /* 若没有线程等待信号量                 */
            psem->value = cnt;
        } else {
             *perr = OS_ERR_TASK_WAITING;             /* 有任务正在等待该信号量,不可以设置value        */
        }
    }
    OS_EXIT_CRITICAL();
}
#endif

#endif                                                /* OS_SEM_EN                                     */
