/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-27     Meco Man     first version
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
*                                      MESSAGE MAILBOX MANAGEMENT
*
* Filename : os_mbox.c
* Version  : V2.93.00
*********************************************************************************************************
*/

#include <ucos_ii.h>

#if OS_MBOX_EN > 0u
/*
*********************************************************************************************************
*                                        ACCEPT MESSAGE FROM MAILBOX
*
* Description: This function checks the mailbox to see if a message is available.  Unlike OSMboxPend(),
*              OSMboxAccept() does not suspend the calling task if a message is not available.
*
* Arguments  : pevent        is a pointer to the event control block
*
* Returns    : != (void *)0  is the message in the mailbox if one is available.  The mailbox is cleared
*                            so the next time OSMboxAccept() is called, the mailbox will be empty.
*              == (void *)0  if the mailbox is empty or,
*                            if 'pevent' is a NULL pointer or,
*                            if you didn't pass the proper event pointer.
*********************************************************************************************************
*/

#if OS_MBOX_ACCEPT_EN > 0u
void  *OSMboxAccept (OS_EVENT *pevent)
{
    INT8U err;
    return OSQAccept(pevent, &err);
}
#endif


/*
*********************************************************************************************************
*                                          CREATE A MESSAGE MAILBOX
*
* Description: This function creates a message mailbox if free event control blocks are available.
*
* Arguments  : pmsg          is a pointer to a message that you wish to deposit in the mailbox.  If
*                            you set this value to the NULL pointer (i.e. (void *)0) then the mailbox
*                            will be considered empty.
*
* Returns    : != (OS_EVENT *)0  is a pointer to the event control clock (OS_EVENT) associated with the
*                                created mailbox
*              == (OS_EVENT *)0  if no event control blocks were available
*********************************************************************************************************
*/

OS_EVENT  *OSMboxCreate (void *pmsg)
{
    OS_EVENT  *pevent;

    pevent = OSQCreate(RT_NULL, 1);
    if(pmsg && pevent) {
        OSQPost(pevent, pmsg);
    }
    return pevent;
}


/*
*********************************************************************************************************
*                                           DELETE A MAIBOX
*
* Description: This function deletes a mailbox and readies all tasks pending on the mailbox.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            mailbox.
*
*              opt           determines delete options as follows:
*                            opt == OS_DEL_NO_PEND   Delete the mailbox ONLY if no task pending
*                            opt == OS_DEL_ALWAYS    Deletes the mailbox even if tasks are waiting.
*                                                    In this case, all the tasks pending will be readied.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            OS_ERR_NONE                  The call was successful and the mailbox was deleted
*                            OS_ERR_DEL_ISR               If you attempted to delete the mailbox from an ISR
*                            OS_ERR_INVALID_OPT           An invalid option was specified
*                            OS_ERR_ILLEGAL_DEL_RUN_TIME  If you tried to delete a mailbox after safety
*                                                         critical operation started.
*                            OS_ERR_TASK_WAITING          One or more tasks were waiting on the mailbox
*                            OS_ERR_EVENT_TYPE            If you didn't pass a pointer to a mailbox
*                            OS_ERR_PEVENT_NULL           If 'pevent' is a NULL pointer.
*
* Returns    : pevent        upon error
*              (OS_EVENT *)0 if the mailbox was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the mailbox MUST check the return code of OSMboxPend().
*              2) OSMboxAccept() callers will not know that the intended mailbox has been deleted!
*              3) This call can potentially disable interrupts for a long time.  The interrupt disable
*                 time is directly proportional to the number of tasks waiting on the mailbox.
*              4) Because ALL tasks pending on the mailbox will be readied, you MUST be careful in
*                 applications where the mailbox is used for mutual exclusion because the resource(s)
*                 will no longer be guarded by the mailbox.
*              5) All tasks that were waiting for the mailbox will be readied and returned an
*                 OS_ERR_PEND_ABORT if OSMboxDel() was called with OS_DEL_ALWAYS
*********************************************************************************************************
*/

#if OS_MBOX_DEL_EN > 0u
OS_EVENT  *OSMboxDel (OS_EVENT  *pevent,
                      INT8U      opt,
                      INT8U     *perr)
{
    return OSQDel(pevent, opt, perr);
}
#endif


/*
*********************************************************************************************************
*                                    PEND ON MAILBOX FOR A MESSAGE
*
* Description: This function waits for a message to be sent to a mailbox
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will
*                            wait for a message to arrive at the mailbox up to the amount of time
*                            specified by this argument.  If you specify 0, however, your task will wait
*                            forever at the specified mailbox or, until a message arrives.
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            OS_ERR_NONE         The call was successful and your task received a
*                                                message.
*                            OS_ERR_TIMEOUT      A message was not received within the specified 'timeout'.
*                            OS_ERR_PEND_ABORT   The wait on the mailbox was aborted.
*                            OS_ERR_EVENT_TYPE   Invalid event type
*                            OS_ERR_PEND_ISR     If you called this function from an ISR and the result
*                                                would lead to a suspension.
*                            OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer
*                            OS_ERR_PEND_LOCKED  If you called this function when the scheduler is locked
*
* Returns    : != (void *)0  is a pointer to the message received
*              == (void *)0  if no message was received or,
*                            if 'pevent' is a NULL pointer or,
*                            if you didn't pass the proper pointer to the event control block.
*********************************************************************************************************
*/

void  *OSMboxPend (OS_EVENT  *pevent,
                   INT32U     timeout,
                   INT8U     *perr)
{
    return OSQPend(pevent, timeout, perr);
}


/*
*********************************************************************************************************
*                                     ABORT WAITING ON A MESSAGE MAILBOX
*
* Description: This function aborts & readies any tasks currently waiting on a mailbox.  This function
*              should be used to fault-abort the wait on the mailbox, rather than to normally signal
*              the mailbox via OSMboxPost() or OSMboxPostOpt().
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox.
*
*              opt           determines the type of ABORT performed:
*                            OS_PEND_OPT_NONE         ABORT wait for a single task (HPT) waiting on the
*                                                     mailbox
*                            OS_PEND_OPT_BROADCAST    ABORT wait for ALL tasks that are  waiting on the
*                                                     mailbox
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            OS_ERR_NONE         No tasks were     waiting on the mailbox.
*                            OS_ERR_PEND_ABORT   At least one task waiting on the mailbox was readied
*                                                and informed of the aborted wait; check return value
*                                                for the number of tasks whose wait on the mailbox
*                                                was aborted.
*                            OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a mailbox.
*                            OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
*
* Returns    : == 0          if no tasks were waiting on the mailbox, or upon error.
*              >  0          if one or more tasks waiting on the mailbox are now readied and informed.
*********************************************************************************************************
*/

#if OS_MBOX_PEND_ABORT_EN > 0u
INT8U  OSMboxPendAbort (OS_EVENT  *pevent,
                        INT8U      opt,
                        INT8U     *perr)
{
    return OSQPendAbort(pevent, opt, perr);
}
#endif


/*
*********************************************************************************************************
*                                      POST MESSAGE TO A MAILBOX
*
* Description: This function sends a message to a mailbox
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox
*
*              pmsg          is a pointer to the message to send.  You MUST NOT send a NULL pointer.
*
* Returns    : OS_ERR_NONE          The call was successful and the message was sent
*              OS_ERR_MBOX_FULL     If the mailbox already contains a message.  You can can only send one
*                                   message at a time and thus, the message MUST be consumed before you
*                                   are allowed to send another one.
*              OS_ERR_EVENT_TYPE    If you are attempting to post to a non mailbox.
*              OS_ERR_PEVENT_NULL   If 'pevent' is a NULL pointer
*              OS_ERR_POST_NULL_PTR If you are attempting to post a NULL pointer
*
* Note(s)    : 1) HPT means Highest Priority Task
*********************************************************************************************************
*/

#if OS_MBOX_POST_EN > 0u
INT8U  OSMboxPost (OS_EVENT  *pevent,
                   void      *pmsg)
{
    return OSQPost(pevent, pmsg);
}
#endif


/*
*********************************************************************************************************
*                                      POST MESSAGE TO A MAILBOX
*
* Description: This function sends a message to a mailbox
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox
*
*              pmsg          is a pointer to the message to send.  You MUST NOT send a NULL pointer.
*
*              opt           determines the type of POST performed:
*                            OS_POST_OPT_NONE         POST to a single waiting task
*                                                     (Identical to OSMboxPost())
*                            OS_POST_OPT_BROADCAST    POST to ALL tasks that are waiting on the mailbox
*
*                            OS_POST_OPT_NO_SCHED     Indicates that the scheduler will NOT be invoked
*
* Returns    : OS_ERR_NONE          The call was successful and the message was sent
*              OS_ERR_MBOX_FULL     If the mailbox already contains a message.  You can can only send one
*                                   message at a time and thus, the message MUST be consumed before you
*                                   are allowed to send another one.
*              OS_ERR_EVENT_TYPE    If you are attempting to post to a non mailbox.
*              OS_ERR_PEVENT_NULL   If 'pevent' is a NULL pointer
*              OS_ERR_POST_NULL_PTR If you are attempting to post a NULL pointer
*
* Note(s)    : 1) HPT means Highest Priority Task
*
* Warning    : Interrupts can be disabled for a long time if you do a 'broadcast'.  In fact, the
*              interrupt disable time is proportional to the number of tasks waiting on the mailbox.
*********************************************************************************************************
*/

#if OS_MBOX_POST_OPT_EN > 0u
INT8U  OSMboxPostOpt (OS_EVENT  *pevent,
                      void      *pmsg,
                      INT8U      opt)
{
    return OSQPostOpt(pevent, pmsg, opt);
}
#endif


/*
*********************************************************************************************************
*                                       QUERY A MESSAGE MAILBOX
*
* Description: This function obtains information about a message mailbox.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox
*
*              p_mbox_data   is a pointer to a structure that will contain information about the message
*                            mailbox.
*
* Returns    : OS_ERR_NONE         The call was successful and the message was sent
*              OS_ERR_EVENT_TYPE   If you are attempting to obtain data from a non mailbox.
*              OS_ERR_PEVENT_NULL  If 'pevent'      is a NULL pointer
*              OS_ERR_PDATA_NULL   If 'p_mbox_data' is a NULL pointer
*********************************************************************************************************
*/

#if OS_MBOX_QUERY_EN > 0u
INT8U  OSMboxQuery (OS_EVENT      *pevent,
                    OS_MBOX_DATA  *p_mbox_data)
{
    INT8U err;
    OS_Q_DATA q_data;

    err = OSQQuery(pevent, &q_data);
    p_mbox_data->OSMsg = q_data.OSMsg;
    return err;
}
#endif                                                     /* OS_MBOX_QUERY_EN                         */
#endif                                                     /* OS_MBOX_EN                               */
