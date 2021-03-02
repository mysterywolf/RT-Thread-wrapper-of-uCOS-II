/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-10     Meco Man     first version
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
*                                        EVENT FLAG  MANAGEMENT
*
* Filename : os_flag.c
* Version  : V2.93.00
*********************************************************************************************************
*/

#include "ucos_ii.h"

#if (OS_FLAG_EN > 0u)

/*
*********************************************************************************************************
*                          CHECK THE STATUS OF FLAGS IN AN EVENT FLAG GROUP
*
* Description: This function is called to check the status of a combination of bits to be set or cleared
*              in an event flag group.  Your application can check for ANY bit to be set/cleared or ALL
*              bits to be set/cleared.
*
*              This call does not block if the desired flags are not present.
*
* Arguments  : pgrp          is a pointer to the desired event flag group.
*
*              flags         Is a bit pattern indicating which bit(s) (i.e. flags) you wish to check.
*                            The bits you want are specified by setting the corresponding bits in
*                            'flags'.  e.g. if your application wants to wait for bits 0 and 1 then
*                            'flags' would contain 0x03.
*
*              wait_type     specifies whether you want ALL bits to be set/cleared or ANY of the bits
*                            to be set/cleared.
*                            You can specify the following argument:
*
*                            OS_FLAG_WAIT_CLR_ALL   You will check ALL bits in 'flags' to be clear (0)
*                            OS_FLAG_WAIT_CLR_ANY   You will check ANY bit  in 'flags' to be clear (0)
*                            OS_FLAG_WAIT_SET_ALL   You will check ALL bits in 'flags' to be set   (1)
*                            OS_FLAG_WAIT_SET_ANY   You will check ANY bit  in 'flags' to be set   (1)
*
*                            NOTE: Add OS_FLAG_CONSUME if you want the event flag to be 'consumed' by
*                                  the call.  Example, to wait for any flag in a group AND then clear
*                                  the flags that are present, set 'wait_type' to:
*
*                                  OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME
*
*              perr          is a pointer to an error code and can be:
*                            OS_ERR_NONE               No error
*                            OS_ERR_EVENT_TYPE         You are not pointing to an event flag group
*                            OS_ERR_FLAG_WAIT_TYPE     You didn't specify a proper 'wait_type' argument.
*                            OS_ERR_FLAG_INVALID_PGRP  You passed a NULL pointer instead of the event flag
*                                                      group handle.
*                            OS_ERR_FLAG_NOT_RDY       The desired flags you are waiting for are not
*                                                      available.
*
* Returns    : The flags in the event flag group that made the task ready or, 0 if a timeout or an error
*              occurred.
*
* Called from: Task or ISR
*
* Note(s)    : 1) IMPORTANT, the behavior of this function has changed from PREVIOUS versions.  The
*                 function NOW returns the flags that were ready INSTEAD of the current state of the
*                 event flags.
*********************************************************************************************************
*/

#if OS_FLAG_ACCEPT_EN > 0u
OS_FLAGS  OSFlagAccept (OS_FLAG_GRP  *pgrp,
                        OS_FLAGS      flags,
                        INT8U         wait_type,
                        INT8U        *perr)
{
    OS_FLAGS      flags_rdy;
    INT8U         result;
    BOOLEAN       consume;
    rt_uint8_t    rt_option = 0;
    rt_err_t      rt_err;


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_FLAGS)0);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pgrp == (OS_FLAG_GRP *)0) {                        /* Validate 'pgrp'                          */
        *perr = OS_ERR_FLAG_INVALID_PGRP;
        return ((OS_FLAGS)0);
    }
#endif
    if (rt_object_get_type(&pgrp->pFlagGrp->parent.parent) /* Validate event block type                */
        != RT_Object_Class_Event) {
        *perr = OS_ERR_EVENT_TYPE;
        return ((OS_FLAGS)0);
    }
    result = (INT8U)(wait_type & OS_FLAG_CONSUME);
    if (result != (INT8U)0) {                              /* See if we need to consume the flags      */
        wait_type &= ~OS_FLAG_CONSUME;
        consume    = OS_TRUE;
    } else {
        consume    = OS_FALSE;
    }

    *perr = OS_ERR_NONE;                                   /* Assume NO error until proven otherwise.  */
    switch (wait_type) {
        case OS_FLAG_WAIT_SET_ALL:                         /* See if all required flags are set        */
            rt_option = RT_EVENT_FLAG_AND;
            break;

        case OS_FLAG_WAIT_SET_ANY:
            rt_option = RT_EVENT_FLAG_OR;
            break;

#if OS_FLAG_WAIT_CLR_EN > 0u
        case OS_FLAG_WAIT_CLR_ALL:                         /* See if all required flags are cleared    */
            rt_option = RT_EVENT_FLAG_AND;
            break;

        case OS_FLAG_WAIT_CLR_ANY:
            rt_option = RT_EVENT_FLAG_OR;
            break;
#endif
        default:
            *perr = OS_ERR_FLAG_WAIT_TYPE;
            return 0;
    }

    if (consume == OS_TRUE){
        rt_option |= RT_EVENT_FLAG_CLEAR;
    }

    rt_err = rt_event_recv(pgrp->pFlagGrp,
                           flags,
                           rt_option,
                           RT_WAITING_NO,
                           &flags_rdy);
    if (rt_err == -RT_ETIMEOUT) {
        *perr = OS_ERR_FLAG_NOT_RDY;
        flags_rdy = 0;
    } else {
        *perr = OS_ERR_NONE;
    }

    return (flags_rdy);
}
#endif


/*
*********************************************************************************************************
*                                        CREATE AN EVENT FLAG
*
* Description: This function is called to create an event flag group.
*
* Arguments  : flags         Contains the initial value to store in the event flag group.
*
*              perr          is a pointer to an error code which will be returned to your application:
*                               OS_ERR_NONE                     if the call was successful.
*                               OS_ERR_CREATE_ISR               if you attempted to create an Event Flag from an
*                                                               ISR.
*                               OS_ERR_FLAG_GRP_DEPLETED        if there are no more event flag groups
*                               OS_ERR_ILLEGAL_CREATE_RUN_TIME  if you tried to create an event flag after
*                                                               safety critical operation started.
*
* Returns    : A pointer to an event flag group or a NULL pointer if no more groups are available.
*
* Called from: Task ONLY
*********************************************************************************************************
*/

OS_FLAG_GRP  *OSFlagCreate (OS_FLAGS  flags,
                            INT8U    *perr)
{
    OS_FLAG_GRP *pgrp;
#if OS_CRITICAL_METHOD == 3u                        /* Allocate storage for CPU status register        */
    OS_CPU_SR    cpu_sr = 0u;
#endif
    rt_event_t rt_event;

#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_FLAG_GRP *)0);
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        *perr = OS_ERR_ILLEGAL_CREATE_RUN_TIME;
        return ((OS_FLAG_GRP *)0);
    }
#endif

    if (OSIntNesting > 0u) {                        /* See if called from ISR ...                      */
        *perr = OS_ERR_CREATE_ISR;                  /* ... can't CREATE from an ISR                    */
        return ((OS_FLAG_GRP *)0);
    }

    pgrp = RT_KERNEL_MALLOC(sizeof(OS_FLAG_GRP));
    if (pgrp == (OS_FLAG_GRP *)0) {                 /* Get an event control block               */
        return ((OS_FLAG_GRP *)0);
    }

    rt_event = rt_event_create("uCOS-II", RT_IPC_FLAG_FIFO);
    if(!rt_event) {
        *perr = OS_ERR_FLAG_GRP_DEPLETED;
        RT_KERNEL_FREE(pgrp);
    } else {
        *perr = OS_ERR_NONE;
    }

    OS_ENTER_CRITICAL();
    pgrp->pFlagGrp    = rt_event;
    pgrp->OSFlagFlags = flags;
    OS_EXIT_CRITICAL();

    return (pgrp);                                  /* Return pointer to event flag group              */
}


/*
*********************************************************************************************************
*                                     DELETE AN EVENT FLAG GROUP
*
* Description: This function deletes an event flag group and readies all tasks pending on the event flag
*              group.
*
* Arguments  : pgrp          is a pointer to the desired event flag group.
*
*              opt           determines delete options as follows:
*                            opt == OS_DEL_NO_PEND   Deletes the event flag group ONLY if no task pending
*                            opt == OS_DEL_ALWAYS    Deletes the event flag group even if tasks are
*                                                    waiting.  In this case, all the tasks pending will be
*                                                    readied.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            OS_ERR_NONE                  The call was successful and the event flag group was
*                                                         deleted
*                            OS_ERR_DEL_ISR               If you attempted to delete the event flag group from
*                                                         an ISR
*                            OS_ERR_FLAG_INVALID_PGRP     If 'pgrp' is a NULL pointer.
*                            OS_ERR_EVENT_TYPE            If you didn't pass a pointer to an event flag group
*                            OS_ERR_ILLEGAL_DEL_RUN_TIME  If you tried to delete an event flag after
*                                                         safety critical operation started.
*                            OS_ERR_INVALID_OPT           An invalid option was specified
*                            OS_ERR_TASK_WAITING          One or more tasks were waiting on the event flag
*                                                         group.
*
* Returns    : pgrp          upon error
*              (OS_EVENT *)0 if the event flag group was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the event flag group MUST check the return code of OSFlagAccept() and OSFlagPend().
*              2) This call can potentially disable interrupts for a long time.  The interrupt disable
*                 time is directly proportional to the number of tasks waiting on the event flag group.
*              3) All tasks that were waiting for the event flag will be readied and returned an
*                 OS_ERR_PEND_ABORT if OSFlagDel() was called with OS_DEL_ALWAYS
*********************************************************************************************************
*/

#if OS_FLAG_DEL_EN > 0u
OS_FLAG_GRP  *OSFlagDel (OS_FLAG_GRP  *pgrp,
                         INT8U         opt,
                         INT8U        *perr)
{
    OS_FLAG_GRP  *pgrp_return;
#if OS_CRITICAL_METHOD == 3u                               /* Allocate storage for CPU status register */
    OS_CPU_SR     cpu_sr = 0u;
#endif


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_FLAG_GRP *)0);
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        *perr = OS_ERR_ILLEGAL_DEL_RUN_TIME;
        return ((OS_FLAG_GRP *)0);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pgrp == (OS_FLAG_GRP *)0) {                        /* Validate 'pgrp'                          */
        *perr = OS_ERR_FLAG_INVALID_PGRP;
        return (pgrp);
    }
#endif

    if (OSIntNesting > 0u) {                               /* See if called from ISR ...               */
        *perr = OS_ERR_DEL_ISR;                            /* ... can't DELETE from an ISR             */
        return (pgrp);
    }
    if (rt_object_get_type(&pgrp->pFlagGrp->parent.parent) /* Validate event block type                */
        != RT_Object_Class_Event) {
        *perr = OS_ERR_EVENT_TYPE;
        return (pgrp);
    }
    OS_ENTER_CRITICAL();
    switch (opt) {
        case OS_DEL_NO_PEND:                               /* Delete group if no task waiting          */
             if (rt_list_isempty(&(pgrp->pFlagGrp->parent.suspend_thread))) { /* 若没有线程等待信号量  */
                 pgrp->OSFlagFlags    = (OS_FLAGS)0;
                 OS_EXIT_CRITICAL();
                 rt_event_detach(pgrp->pFlagGrp);
                 RT_KERNEL_FREE(pgrp);
                 *perr                = OS_ERR_NONE;
                 pgrp_return          = (OS_FLAG_GRP *)0;  /* Event Flag Group has been deleted        */
             } else {
                 OS_EXIT_CRITICAL();
                 *perr                = OS_ERR_TASK_WAITING;
                 pgrp_return          = pgrp;
             }
             break;

        case OS_DEL_ALWAYS:                                /* Always delete the event flag group       */
             pgrp->OSFlagFlags    = (OS_FLAGS)0;
             OS_EXIT_CRITICAL();
             rt_event_detach(pgrp->pFlagGrp);
             RT_KERNEL_FREE(pgrp);
             *perr = OS_ERR_NONE;
             pgrp_return          = (OS_FLAG_GRP *)0;      /* Event Flag Group has been deleted        */
             break;

        default:
             OS_EXIT_CRITICAL();
             *perr                = OS_ERR_INVALID_OPT;
             pgrp_return          = pgrp;
             break;
    }

    return (pgrp_return);
}
#endif



/*
*********************************************************************************************************
*                                ASSIGN A NAME TO AN EVENT FLAG GROUP
*
* Description: This function assigns a name to an event flag group.
*
* Arguments  : pgrp      is a pointer to the event flag group.
*
*              pname     is a pointer to an ASCII string that will be used as the name of the event flag
*                        group.
*
*              perr      is a pointer to an error code that can contain one of the following values:
*
*                        OS_ERR_NONE                if the requested task is resumed
*                        OS_ERR_EVENT_TYPE          if 'pevent' is not pointing to an event flag group
*                        OS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        OS_ERR_FLAG_INVALID_PGRP   if you passed a NULL pointer for 'pgrp'
*                        OS_ERR_NAME_SET_ISR        if you called this function from an ISR
*
* Returns    : None
*********************************************************************************************************
*/

#if OS_FLAG_NAME_EN > 0u
void  OSFlagNameSet (OS_FLAG_GRP  *pgrp,
                     INT8U        *pname,
                     INT8U        *perr)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif



#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pgrp == (OS_FLAG_GRP *)0) {              /* Is 'pgrp' a NULL pointer?                          */
        *perr = OS_ERR_FLAG_INVALID_PGRP;
        return;
    }
    if (pname == (INT8U *)0) {                   /* Is 'pname' a NULL pointer?                         */
        *perr = OS_ERR_PNAME_NULL;
        return;
    }
#endif
    if (OSIntNesting > 0u) {                     /* See if trying to call from an ISR                  */
        *perr = OS_ERR_NAME_SET_ISR;
        return;
    }
    OS_ENTER_CRITICAL();
    if (pgrp->OSFlagType != OS_EVENT_TYPE_FLAG) {
        OS_EXIT_CRITICAL();
        *perr = OS_ERR_EVENT_TYPE;
        return;
    }
    pgrp->OSFlagName = pname;
    OS_EXIT_CRITICAL();
    OS_TRACE_EVENT_NAME_SET(pgrp, pname);
    *perr            = OS_ERR_NONE;
    return;
}
#endif


/*
*********************************************************************************************************
*                                     WAIT ON AN EVENT FLAG GROUP
*
* Description: This function is called to wait for a combination of bits to be set in an event flag
*              group.  Your application can wait for ANY bit to be set or ALL bits to be set.
*
* Arguments  : pgrp          is a pointer to the desired event flag group.
*
*              flags         Is a bit pattern indicating which bit(s) (i.e. flags) you wish to wait for.
*                            The bits you want are specified by setting the corresponding bits in
*                            'flags'.  e.g. if your application wants to wait for bits 0 and 1 then
*                            'flags' would contain 0x03.
*
*              wait_type     specifies whether you want ALL bits to be set or ANY of the bits to be set.
*                            You can specify the following argument:
*
*                            OS_FLAG_WAIT_CLR_ALL   You will wait for ALL bits in 'mask' to be clear (0)
*                            OS_FLAG_WAIT_SET_ALL   You will wait for ALL bits in 'mask' to be set   (1)
*                            OS_FLAG_WAIT_CLR_ANY   You will wait for ANY bit  in 'mask' to be clear (0)
*                            OS_FLAG_WAIT_SET_ANY   You will wait for ANY bit  in 'mask' to be set   (1)
*
*                            NOTE: Add OS_FLAG_CONSUME if you want the event flag to be 'consumed' by
*                                  the call.  Example, to wait for any flag in a group AND then clear
*                                  the flags that are present, set 'wait_type' to:
*
*                                  OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME
*
*              timeout       is an optional timeout (in clock ticks) that your task will wait for the
*                            desired bit combination.  If you specify 0, however, your task will wait
*                            forever at the specified event flag group or, until a message arrives.
*
*              perr          is a pointer to an error code and can be:
*                            OS_ERR_NONE               The desired bits have been set within the specified
*                                                      'timeout'.
*                            OS_ERR_PEND_ISR           If you tried to PEND from an ISR
*                            OS_ERR_FLAG_INVALID_PGRP  If 'pgrp' is a NULL pointer.
*                            OS_ERR_EVENT_TYPE         You are not pointing to an event flag group
*                            OS_ERR_TIMEOUT            The bit(s) have not been set in the specified
*                                                      'timeout'.
*                            OS_ERR_PEND_ABORT         The wait on the flag was aborted.
*                            OS_ERR_FLAG_WAIT_TYPE     You didn't specify a proper 'wait_type' argument.
*
* Returns    : The flags in the event flag group that made the task ready or, 0 if a timeout or an error
*              occurred.
*
* Called from: Task ONLY
*
* Note(s)    : 1) IMPORTANT, the behavior of this function has changed from PREVIOUS versions.  The
*                 function NOW returns the flags that were ready INSTEAD of the current state of the
*                 event flags.
*********************************************************************************************************
*/

OS_FLAGS  OSFlagPend (OS_FLAG_GRP  *pgrp,
                      OS_FLAGS      flags,
                      INT8U         wait_type,
                      INT32U        timeout,
                      INT8U        *perr)
{
#if OS_CRITICAL_METHOD == 3u                               /* Allocate storage for CPU status register */
    OS_CPU_SR     cpu_sr = 0u;
#endif
    rt_err_t      rt_err;
    INT8U         result;
    BOOLEAN       consume;
    rt_uint8_t    rt_option = 0;
    OS_FLAGS      flags_rdy;


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_FLAGS)0);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pgrp == (OS_FLAG_GRP *)0) {                        /* Validate 'pgrp'                          */
        *perr = OS_ERR_FLAG_INVALID_PGRP;
        return ((OS_FLAGS)0);
    }
#endif

    if (OSIntNesting > 0u) {                               /* See if called from ISR ...               */
        *perr = OS_ERR_PEND_ISR;                           /* ... can't PEND from an ISR               */
        return ((OS_FLAGS)0);
    }
    if (OSLockNesting > 0u) {                              /* See if called with scheduler locked ...  */
        *perr = OS_ERR_PEND_LOCKED;                        /* ... can't PEND when locked               */
        return ((OS_FLAGS)0);
    }
    if (rt_object_get_type(&pgrp->pFlagGrp->parent.parent) /* Validate event block type                */
        != RT_Object_Class_Event) {
        *perr = OS_ERR_EVENT_TYPE;
        return ((OS_FLAGS)0);
    }

    result = (INT8U)(wait_type & OS_FLAG_CONSUME);
    if (result != (INT8U)0) {                              /* See if we need to consume the flags      */
        wait_type &= (INT8U)~(INT8U)OS_FLAG_CONSUME;
        consume    = OS_TRUE;
    } else {
        consume    = OS_FALSE;
    }

    *perr = OS_ERR_NONE;                                   /* Assume NO error until proven otherwise.  */
    switch (wait_type) {
        case OS_FLAG_WAIT_SET_ALL:                         /* See if all required flags are set        */
            rt_option = RT_EVENT_FLAG_AND;
            break;

        case OS_FLAG_WAIT_SET_ANY:
            rt_option = RT_EVENT_FLAG_OR;
            break;

#if OS_FLAG_WAIT_CLR_EN > 0u
        case OS_FLAG_WAIT_CLR_ALL:                         /* See if all required flags are cleared    */
            rt_option = RT_EVENT_FLAG_AND;
            break;

        case OS_FLAG_WAIT_CLR_ANY:
            rt_option = RT_EVENT_FLAG_OR;
            break;
#endif
        default:
            *perr = OS_ERR_FLAG_WAIT_TYPE;
            return 0;
    }

    if (consume == OS_TRUE){
        rt_option |= RT_EVENT_FLAG_CLEAR;
    }

    OS_ENTER_CRITICAL();                              /* Otherwise, must wait until event occurs       */
    OSTCBCur->OSTCBStat     |= OS_STAT_FLAG;          /* Resource not available, pend on semaphore     */
    OSTCBCur->OSTCBStatPend  = OS_STAT_PEND_OK;
#ifndef PKG_USING_UCOSII_WRAPPER_TINY
    OSTCBCur->OSTCBDly       = timeout;               /* Store pend timeout in TCB                     */
    OSTCBCur->OSTCBFlagsRdy  = rt_thread_self()->event_set;
#endif
    OS_EXIT_CRITICAL();

    if(timeout) {                                     /* 0为永久等待                                   */
        rt_err = rt_event_recv(pgrp->pFlagGrp,
                               flags,
                               rt_option,
                               timeout,
                               &flags_rdy);
        if (rt_err == -RT_ETIMEOUT) {
            *perr = OS_ERR_TIMEOUT;
        } else {
            *perr = OS_ERR_NONE;
        }
    } else {
        rt_err = rt_event_recv(pgrp->pFlagGrp,
                               flags,
                               rt_option,
                               RT_WAITING_FOREVER,
                               &flags_rdy);
        *perr = OS_ERR_NONE;
    }

    OS_ENTER_CRITICAL();
#ifndef PKG_USING_UCOSII_WRAPPER_TINY
    OSTCBCur->OSTCBFlagsRdy      = rt_thread_self()->event_set;
#endif
    OSTCBCur->OSTCBStat          =  OS_STAT_RDY;      /* Set   task  status to ready                   */
    OS_EXIT_CRITICAL();

    return (flags_rdy);
}


/*
*********************************************************************************************************
*                              GET FLAGS WHO CAUSED TASK TO BECOME READY
*
* Description: This function is called to obtain the flags that caused the task to become ready to run.
*              In other words, this function allows you to tell "Who done it!".
*
* Arguments  : None
*
* Returns    : The flags that caused the task to be ready.
*
* Called from: Task ONLY
*********************************************************************************************************
*/

OS_FLAGS  OSFlagPendGetFlagsRdy (void)
{
    OS_FLAGS      flags;
#if OS_CRITICAL_METHOD == 3u                               /* Allocate storage for CPU status register */
    OS_CPU_SR     cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL();
    flags = rt_thread_self()->event_set;
#ifndef PKG_USING_UCOSII_WRAPPER_TINY
    OSTCBCur->OSTCBFlagsRdy = flags;
#endif
    OS_EXIT_CRITICAL();
    return (flags);
}


/*
*********************************************************************************************************
*                                       POST EVENT FLAG BIT(S)
*
* Description: This function is called to set or clear some bits in an event flag group.  The bits to
*              set or clear are specified by a 'bit mask'.
*
* Arguments  : pgrp          is a pointer to the desired event flag group.
*
*              flags         If 'opt' (see below) is OS_FLAG_SET, each bit that is set in 'flags' will
*                            set the corresponding bit in the event flag group.  e.g. to set bits 0, 4
*                            and 5 you would set 'flags' to:
*
*                                0x31     (note, bit 0 is least significant bit)
*
*                            If 'opt' (see below) is OS_FLAG_CLR, each bit that is set in 'flags' will
*                            CLEAR the corresponding bit in the event flag group.  e.g. to clear bits 0,
*                            4 and 5 you would specify 'flags' as:
*
*                                0x31     (note, bit 0 is least significant bit)
*
*              opt           indicates whether the flags will be:
*                                set     (OS_FLAG_SET) or
*                                cleared (OS_FLAG_CLR)
*
*              perr          is a pointer to an error code and can be:
*                            OS_ERR_NONE                The call was successfull
*                            OS_ERR_FLAG_INVALID_PGRP   You passed a NULL pointer
*                            OS_ERR_EVENT_TYPE          You are not pointing to an event flag group
*                            OS_ERR_FLAG_INVALID_OPT    You specified an invalid option
*
* Returns    : the new value of the event flags bits that are still set.
*
* Called From: Task or ISR
*
* WARNING(s) : 1) The execution time of this function depends on the number of tasks waiting on the event
*                 flag group.
*              2) The amount of time interrupts are DISABLED depends on the number of tasks waiting on
*                 the event flag group.
*********************************************************************************************************
*/
OS_FLAGS  OSFlagPost (OS_FLAG_GRP  *pgrp,
                      OS_FLAGS      flags,
                      INT8U         opt,
                      INT8U        *perr)
{
    rt_err_t      rt_err;
#if OS_CRITICAL_METHOD == 3u                         /* Allocate storage for CPU status register       */
    OS_CPU_SR     cpu_sr = 0u;
#endif


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_FLAGS)0);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pgrp == (OS_FLAG_GRP *)0) {                  /* Validate 'pgrp'                                */
        *perr = OS_ERR_FLAG_INVALID_PGRP;
        return ((OS_FLAGS)0);
    }
#endif

    if (rt_object_get_type(&pgrp->pFlagGrp->parent.parent) /* Validate event block type                */
        != RT_Object_Class_Event) {
        *perr = OS_ERR_EVENT_TYPE;
        return ((OS_FLAGS)0);
    }

    rt_err = rt_event_send(pgrp->pFlagGrp, flags);
    if(rt_err == RT_EOK) {
        *perr = OS_ERR_NONE;
    } else {
        *perr = OS_ERR_FLAG_INVALID_PGRP;
    }

    OS_ENTER_CRITICAL();
#ifndef PKG_USING_UCOSII_WRAPPER_TINY
    OSTCBCur->OSTCBFlagsRdy = rt_thread_self()->event_set;
#endif
    flags = pgrp->pFlagGrp->set;
    OS_EXIT_CRITICAL();
    return (flags);
}


/*
*********************************************************************************************************
*                                          QUERY EVENT FLAG
*
* Description: This function is used to check the value of the event flag group.
*
* Arguments  : pgrp         is a pointer to the desired event flag group.
*
*              perr          is a pointer to an error code returned to the called:
*                            OS_ERR_NONE                The call was successfull
*                            OS_ERR_FLAG_INVALID_PGRP   You passed a NULL pointer
*                            OS_ERR_EVENT_TYPE          You are not pointing to an event flag group
*
* Returns    : The current value of the event flag group.
*
* Called From: Task or ISR
*********************************************************************************************************
*/

#if OS_FLAG_QUERY_EN > 0u
OS_FLAGS  OSFlagQuery (OS_FLAG_GRP  *pgrp,
                       INT8U        *perr)
{
    OS_FLAGS   flags;
#if OS_CRITICAL_METHOD == 3u                      /* Allocate storage for CPU status register          */
    OS_CPU_SR  cpu_sr = 0u;
#endif


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_FLAGS)0);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pgrp == (OS_FLAG_GRP *)0) {               /* Validate 'pgrp'                                   */
        *perr = OS_ERR_FLAG_INVALID_PGRP;
        return ((OS_FLAGS)0);
    }
#endif
    if (rt_object_get_type(&pgrp->pFlagGrp->parent.parent) /* Validate event block type                */
        != RT_Object_Class_Event) {
        *perr = OS_ERR_EVENT_TYPE;
        return ((OS_FLAGS)0);
    }
    OS_ENTER_CRITICAL();
    flags = pgrp->pFlagGrp->set;
    OS_EXIT_CRITICAL();
    *perr = OS_ERR_NONE;
    return (flags);                               /* Return the current value of the event flags       */
}
#endif

#endif
