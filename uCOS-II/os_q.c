/*
 * Copyright (c) 2021, Meco Jianting Man <jiantingman@foxmail.com>
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
*                                       MESSAGE QUEUE MANAGEMENT
*
* Filename : os_q.c
* Version  : V2.93.00
*********************************************************************************************************
*/

#include "ucos_ii.h"

#if (OS_Q_EN > 0u)

/*
*********************************************************************************************************
*                                      ACCEPT MESSAGE FROM QUEUE
*
* Description: This function checks the queue to see if a message is available.  Unlike OSQPend(),
*              OSQAccept() does not suspend the calling task if a message is not available.
*
* Arguments  : pevent        is a pointer to the event control block
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            OS_ERR_NONE         The call was successful and your task received a
*                                                message.
*                            OS_ERR_EVENT_TYPE   You didn't pass a pointer to a queue
*                            OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer
*                            OS_ERR_Q_EMPTY      The queue did not contain any messages
*
* Returns    : != (void *)0  is the message in the queue if one is available.  The message is removed
*                            from the so the next time OSQAccept() is called, the queue will contain
*                            one less entry.
*              == (void *)0  if you received a NULL pointer message
*                            if the queue is empty or,
*                            if 'pevent' is a NULL pointer or,
*                            if you passed an invalid event type
*
* Note(s)    : As of V2.60, you can now pass NULL pointers through queues.  Because of this, the argument
*              'perr' has been added to the API to tell you about the outcome of the call.
*********************************************************************************************************
*/

#if OS_Q_ACCEPT_EN > 0u
void  *OSQAccept (OS_EVENT  *pevent,
                  INT8U     *perr)
{
    rt_mq_t    pmq;
    rt_err_t   rt_err;
    void      *pmsg;
    ucos_msg_t ucos_msg;

#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((void *)0);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {               /* Validate 'pevent'                                  */
        *perr = OS_ERR_PEVENT_NULL;
        return ((void *)0);
    }
#endif

    pmq = (rt_mq_t)pevent->ipc_ptr;

    if (rt_object_get_type(&pmq->parent.parent)  /* Validate event block type                     */
        != RT_Object_Class_MessageQueue) {
        *perr = OS_ERR_EVENT_TYPE;
        return ((void *)0);
    }

    rt_err = rt_mq_recv(pmq,                     /* invoke rt-thread API                               */
                 (void*)&ucos_msg,               /* uCOS消息段                                         */
                 sizeof(ucos_msg_t),             /* uCOS消息段长度                                     */
                 RT_WAITING_NO);                 /* 非阻塞                                             */
    if(rt_err == RT_EOK) {                       /* See if any messages in the queue                   */
        *perr = OS_ERR_NONE;
        pmsg =  ucos_msg.data_ptr;               /* Yes, extract oldest message from the queue         */
    } else {
        *perr = OS_ERR_Q_EMPTY;
        pmsg = (void *)0;                        /* Queue is empty                                     */
    }
    return (pmsg);                               /* Return message received (or NULL)                  */
}
#endif


/*
*********************************************************************************************************
*                                       CREATE A MESSAGE QUEUE
*
* Description: This function creates a message queue if free event control blocks are available.
*
* Arguments  : start         is a pointer to the base address of the message queue storage area.  The
*                            storage area MUST be declared as an array of pointers to 'void' as follows
*                            在原版uCOS-II中,用于用户指定消息内存池指针,但是rt_mq_create内部会自动创建消息
*                            内存池,因此该参数直接填NULL即可
*
*                            void *MessageStorage[size]
*
*              size          is the number of elements in the storage area
*
* Returns    : != (OS_EVENT *)0  is a pointer to the event control clock (OS_EVENT) associated with the
*                                created queue
*              == (OS_EVENT *)0  if no event control blocks were available or an error was detected
*********************************************************************************************************
*/

OS_EVENT  *OSQCreate (void    **start,
                      INT16U    size)
{
    OS_EVENT  *pevent;

    (void)start;

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_EVENT *)0);
    }
#endif

    if (OSIntNesting > 0u) {                     /* See if called from ISR ...                         */
        return ((OS_EVENT *)0);                  /* ... can't CREATE from an ISR                       */
    }

    pevent = RT_KERNEL_MALLOC(sizeof(OS_EVENT));
    if (pevent == (OS_EVENT *)0) {               /* See if we have an event control block              */
        return ((OS_EVENT *)0);
    }

    pevent->ipc_ptr = (struct rt_ipc_object *)   /* invoke rt-thread API to create message queue       */
        rt_mq_create("uCOS-II", sizeof(rt_ubase_t), size, RT_IPC_FLAG_PRIO);
    if(!pevent->ipc_ptr) {
        RT_KERNEL_FREE(pevent);
        return ((OS_EVENT *)0);
    }

    return (pevent);
}


/*
*********************************************************************************************************
*                                       CREATE A MESSAGE QUEUE
*   额外实现OSQCreateEx()函数，该函数并不在uCOS-II原版的函数中，OSQCreateEx()函数中第一个参数size在
* 本兼容层中没有意义，因此该函数将OSQCreateEx()函数中的第一个参数略去，以方便用户使用。
*   推荐用户使用这个API
*********************************************************************************************************
*/
OS_EVENT  *OSQCreateEx (INT16U    size)
{
    return OSQCreate(0, size);                   /* 缓冲池首地址无需给出,在兼容层中可以随便给一个即可  */
}


/*
*********************************************************************************************************
*                                       DELETE A MESSAGE QUEUE
*
* Description: This function deletes a message queue and readies all tasks pending on the queue.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            queue.
*
*              opt           determines delete options as follows:
*                            opt == OS_DEL_NO_PEND   Delete the queue ONLY if no task pending
*                            opt == OS_DEL_ALWAYS    Deletes the queue even if tasks are waiting.
*                                                    In this case, all the tasks pending will be readied.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            OS_ERR_NONE                  The call was successful and the queue was deleted
*                            OS_ERR_DEL_ISR               If you tried to delete the queue from an ISR
*                            OS_ERR_ILLEGAL_DEL_RUN_TIME  If you tried to delete the queue after safety
*                                                         critical operation started.
*                            OS_ERR_INVALID_OPT           An invalid option was specified
*                            OS_ERR_TASK_WAITING          One or more tasks were waiting on the queue
*                            OS_ERR_EVENT_TYPE            If you didn't pass a pointer to a queue
*                            OS_ERR_PEVENT_NULL           If 'pevent' is a NULL pointer.
*
* Returns    : pevent        upon error
*              (OS_EVENT *)0 if the queue was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the queue MUST check the return code of OSQPend().
*              2) OSQAccept() callers will not know that the intended queue has been deleted unless
*                 they check 'pevent' to see that it's a NULL pointer.
*              3) This call can potentially disable interrupts for a long time.  The interrupt disable
*                 time is directly proportional to the number of tasks waiting on the queue.
*              4) Because ALL tasks pending on the queue will be readied, you MUST be careful in
*                 applications where the queue is used for mutual exclusion because the resource(s)
*                 will no longer be guarded by the queue.
*              5) If the storage for the message queue was allocated dynamically (i.e. using a malloc()
*                 type call) then your application MUST release the memory storage by call the counterpart
*                 call of the dynamic allocation scheme used.  If the queue storage was created statically
*                 then, the storage can be reused.
*              6) All tasks that were waiting for the queue will be readied and returned an
*                 OS_ERR_PEND_ABORT if OSQDel() was called with OS_DEL_ALWAYS
*********************************************************************************************************
*/

#if OS_Q_DEL_EN > 0u
OS_EVENT  *OSQDel (OS_EVENT  *pevent,
                   INT8U      opt,
                   INT8U     *perr)
{
    OS_EVENT  *pevent_return;
    rt_mq_t    pmq;

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

    pmq = (rt_mq_t)pevent->ipc_ptr;

    if (rt_object_get_type(&pmq->parent.parent)            /* Validate event block type                */
        != RT_Object_Class_MessageQueue) {
       *perr = OS_ERR_EVENT_TYPE;
        return (pevent);
    }

    if (OSIntNesting > 0u) {                               /* See if called from ISR ...               */
        *perr = OS_ERR_DEL_ISR;                            /* ... can't DELETE from an ISR             */
        return (pevent);
    }

    switch (opt) {
        case OS_DEL_NO_PEND:                               /* Delete queue only if no task waiting     */
            if(rt_list_isempty(&(pmq->parent.suspend_thread))) { /* 若没有线程等待信号量               */
                rt_mq_delete(pmq);                         /* invoke RT-Thread API                     */
                RT_KERNEL_FREE(pevent);
                *perr = OS_ERR_NONE;
                pevent_return =  (OS_EVENT *)0;
            } else {
                *perr = OS_ERR_TASK_WAITING;
                pevent_return = pevent;
            }
            break;

        case OS_DEL_ALWAYS:                                /* Always delete the queue                  */
            rt_mq_delete(pmq);                             /* invoke RT-Thread API                     */
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
*                                             FLUSH QUEUE
*
* Description : This function is used to flush the contents of the message queue.
*
* Arguments   : none
*
* Returns     : OS_ERR_NONE         upon success
*               OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a queue
*               OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer
*
* WARNING     : You should use this function with great care because, when to flush the queue, you LOOSE
*               the references to what the queue entries are pointing to and thus, you could cause
*               'memory leaks'.  In other words, the data you are pointing to that's being referenced
*               by the queue entries should, most likely, need to be de-allocated (i.e. freed).
*********************************************************************************************************
*/

#if OS_Q_FLUSH_EN > 0u
INT8U  OSQFlush (OS_EVENT *pevent)
{
    rt_mq_t    pmq;
    struct _rt_mq_message *msg;
#if OS_CRITICAL_METHOD == 3u                          /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0u;
#endif

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        return (OS_ERR_PEVENT_NULL);
    }
#endif

    pmq = (rt_mq_t)pevent->ipc_ptr;

    if (rt_object_get_type(&pmq->parent.parent)       /* Validate event block type                     */
        != RT_Object_Class_MessageQueue) {
        return (OS_ERR_EVENT_TYPE);
    }

    OS_ENTER_CRITICAL();
    while(pmq->entry>0)
    {
        /* 实现参见了rt_mq_recv函数 */
        msg = (struct _rt_mq_message *)(pmq->msg_queue_head);/* get message from queue                 */
        pmq->msg_queue_head = msg->next;              /* move message queue head                       */
        if (pmq->msg_queue_tail == msg)               /* reach queue tail, set to NULL                 */
            pmq->msg_queue_tail = RT_NULL;
        pmq->entry --;                                /* decrease message entry                        */
        msg->next = (struct _rt_mq_message *)pmq->msg_queue_free; /* put message to free list          */
        pmq->msg_queue_free = msg;
    }
    OS_EXIT_CRITICAL();
    return (OS_ERR_NONE);
}
#endif


/*
*********************************************************************************************************
*                                    PEND ON A QUEUE FOR A MESSAGE
*
* Description: This function waits for a message to be sent to a queue
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired queue
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will
*                            wait for a message to arrive at the queue up to the amount of time
*                            specified by this argument.  If you specify 0, however, your task will wait
*                            forever at the specified queue or, until a message arrives.
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            OS_ERR_NONE         The call was successful and your task received a
*                                                message.
*                            OS_ERR_TIMEOUT      A message was not received within the specified 'timeout'.
*                            OS_ERR_PEND_ABORT   The wait on the queue was aborted.
*                            OS_ERR_EVENT_TYPE   You didn't pass a pointer to a queue
*                            OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer
*                            OS_ERR_PEND_ISR     If you called this function from an ISR and the result
*                                                would lead to a suspension.
*                            OS_ERR_PEND_LOCKED  If you called this function with the scheduler is locked
*
* Returns    : != (void *)0  is a pointer to the message received
*              == (void *)0  if you received a NULL pointer message or,
*                            if no message was received or,
*                            if 'pevent' is a NULL pointer or,
*                            if you didn't pass a pointer to a queue.
*
* Note(s)    : As of V2.60, this function allows you to receive NULL pointer messages.
*********************************************************************************************************
*/

void  *OSQPend (OS_EVENT  *pevent,
                INT32U     timeout,
                INT8U     *perr)
{
    void      *pmsg;
    rt_mq_t    pmq;
    rt_err_t   rt_err;
    ucos_msg_t ucos_msg;
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((void *)0);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {               /* Validate 'pevent'                                  */
        *perr = OS_ERR_PEVENT_NULL;
        return ((void *)0);
    }
#endif

    pmq = (rt_mq_t)pevent->ipc_ptr;

    if (rt_object_get_type(&pmq->parent.parent)  /* Validate event block type                          */
        != RT_Object_Class_MessageQueue) {
        *perr = OS_ERR_EVENT_TYPE;
        return (0u);
    }
    if (OSIntNesting > 0u) {                     /* See if called from ISR ...                         */
        *perr = OS_ERR_PEND_ISR;                 /* ... can't PEND from an ISR                         */
        return ((void *)0);
    }
    if (OSLockNesting > 0u) {                    /* See if called with scheduler locked ...            */
        *perr = OS_ERR_PEND_LOCKED;              /* ... can't PEND when locked                         */
        return ((void *)0);
    }

    OS_ENTER_CRITICAL();
    OSTCBCur->OSTCBStat     |= OS_STAT_Q;        /* Task will have to pend for a message to be posted  */
    OSTCBCur->OSTCBStatPend  = OS_STAT_PEND_OK;
#ifndef PKG_USING_UCOSII_WRAPPER_TINY
    OSTCBCur->OSTCBDly       = timeout;          /* Load timeout into TCB                              */
    OSTCBCur->OSTCBEventPtr  = pevent;
#endif
    OS_EXIT_CRITICAL();

    if(timeout) {
        rt_err = rt_mq_recv(pmq,                          /* invoke rt-thread API                          */
                     (void*)&ucos_msg,                    /* uCOS消息段                                    */
                     sizeof(ucos_msg_t),                  /* uCOS消息段长度                                */
                     timeout);
        OS_ENTER_CRITICAL();
        if (rt_err == RT_EOK) {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_OK;
        } else if(OSTCBCur->OSTCBStatPend == OS_STAT_PEND_ABORT) {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_ABORT;
        } else {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_TO;
        }
    } else {
        rt_mq_recv (pmq,                                  /* invoke rt-thread API                          */
                     (void*)&ucos_msg,                    /* uCOS消息段                                    */
                     sizeof(ucos_msg_t),                  /* uCOS消息段长度                                */
                     RT_WAITING_FOREVER);
        OS_ENTER_CRITICAL();
        if(OSTCBCur->OSTCBStatPend == OS_STAT_PEND_ABORT) {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_ABORT;
        }else {
            OSTCBCur->OSTCBStatPend = OS_STAT_PEND_OK;
        }
    }
    switch (OSTCBCur->OSTCBStatPend) {                    /* See if we timed-out or aborted                */
        case OS_STAT_PEND_OK:                             /* Extract message from TCB (Put there by QPost) */
             pmsg =  ucos_msg.data_ptr;
            *perr =  OS_ERR_NONE;
             break;

        case OS_STAT_PEND_ABORT:
             pmsg = (void *)0;
            *perr =  OS_ERR_PEND_ABORT;                   /* Indicate that we aborted                      */
             break;

        case OS_STAT_PEND_TO:
        default:
             pmsg = (void *)0;
            *perr =  OS_ERR_TIMEOUT;                      /* Indicate that we didn't get event within TO   */
             break;
    }
    OSTCBCur->OSTCBStat          =  OS_STAT_RDY;          /* Set   task  status to ready                   */
    OSTCBCur->OSTCBStatPend      =  OS_STAT_PEND_OK;      /* Clear pend  status                            */
#ifndef PKG_USING_UCOSII_WRAPPER_TINY
    OSTCBCur->OSTCBEventPtr      = (OS_EVENT  *)0;        /* Clear event pointers                          */
#endif
    OS_EXIT_CRITICAL();

    return (pmsg);                                        /* Return received message                       */
}


/*
*********************************************************************************************************
*                                  ABORT WAITING ON A MESSAGE QUEUE
*
* Description: This function aborts & readies any tasks currently waiting on a queue.  This function
*              should be used to fault-abort the wait on the queue, rather than to normally signal
*              the queue via OSQPost(), OSQPostFront() or OSQPostOpt().
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired queue.
*
*              opt           determines the type of ABORT performed:
*                            OS_PEND_OPT_NONE         ABORT wait for a single task (HPT) waiting on the
*                                                     queue
*                            OS_PEND_OPT_BROADCAST    ABORT wait for ALL tasks that are  waiting on the
*                                                     queue
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            OS_ERR_NONE         No tasks were     waiting on the queue.
*                            OS_ERR_PEND_ABORT   At least one task waiting on the queue was readied
*                                                and informed of the aborted wait; check return value
*                                                for the number of tasks whose wait on the queue
*                                                was aborted.
*                            OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a queue.
*                            OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
*
* Returns    : == 0          if no tasks were waiting on the queue, or upon error.
*              >  0          if one or more tasks waiting on the queue are now readied and informed.
*********************************************************************************************************
*/

#if OS_Q_PEND_ABORT_EN > 0u
INT8U  OSQPendAbort (OS_EVENT  *pevent,
                     INT8U      opt,
                     INT8U     *perr)
{
    INT8U      nbr_tasks;
    rt_mq_t    pmq;


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                         /* Validate 'pevent'                        */
        *perr = OS_ERR_PEVENT_NULL;
        return (0u);
    }
#endif

    pmq = (rt_mq_t)pevent->ipc_ptr;

    if (rt_object_get_type(&pmq->parent.parent)            /* Validate event block type                          */
        != RT_Object_Class_MessageQueue) {
        *perr = OS_ERR_EVENT_TYPE;
        return (0u);
    }

    switch (opt)
    {
        case OS_ERR_PEND_ABORT:
            nbr_tasks = rt_ipc_pend_abort_all(&(pmq->parent.suspend_thread));

        case OS_PEND_OPT_NONE:
        default:
            rt_ipc_pend_abort_1(&(pmq->parent.suspend_thread));
            nbr_tasks = 1u;
    }

    *perr = OS_ERR_NONE;
    return nbr_tasks;                                      /* No tasks waiting on queue                */
}
#endif


/*
*********************************************************************************************************
*                                       POST MESSAGE TO A QUEUE
*
* Description: This function sends a message to a queue
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired queue
*
*              pmsg          is a pointer to the message to send.
*
* Returns    : OS_ERR_NONE           The call was successful and the message was sent
*              OS_ERR_Q_FULL         If the queue cannot accept any more messages because it is full.
*              OS_ERR_EVENT_TYPE     If you didn't pass a pointer to a queue.
*              OS_ERR_PEVENT_NULL    If 'pevent' is a NULL pointer
*
* Note(s)    : As of V2.60, this function allows you to send NULL pointer messages.
*********************************************************************************************************
*/

#if OS_Q_POST_EN > 0u
INT8U  OSQPost (OS_EVENT  *pevent,
                void      *pmsg)
{
    rt_mq_t    pmq;
    rt_err_t   rt_err;
    ucos_msg_t ucos_msg;

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                     /* Validate 'pevent'                            */
        return (OS_ERR_PEVENT_NULL);
    }
#endif

    pmq = (rt_mq_t)pevent->ipc_ptr;

    if (rt_object_get_type(&pmq->parent.parent)        /* Validate event block type                    */
        != RT_Object_Class_MessageQueue) {
        return (OS_ERR_EVENT_TYPE);
    }

    /*装填uCOS消息段*/
    ucos_msg.data_ptr = pmsg;
    rt_err = rt_mq_send(pmq,(void*)&ucos_msg,sizeof(ucos_msg_t));
    if(rt_err == -RT_EFULL) {
        return (OS_ERR_Q_FULL);
    }

    return (OS_ERR_NONE);
}
#endif


/*
*********************************************************************************************************
*                                POST MESSAGE TO THE FRONT OF A QUEUE
*
* Description: This function sends a message to a queue but unlike OSQPost(), the message is posted at
*              the front instead of the end of the queue.  Using OSQPostFront() allows you to send
*              'priority' messages.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired queue
*
*              pmsg          is a pointer to the message to send.
*
* Returns    : OS_ERR_NONE           The call was successful and the message was sent
*              OS_ERR_Q_FULL         If the queue cannot accept any more messages because it is full.
*              OS_ERR_EVENT_TYPE     If you didn't pass a pointer to a queue.
*              OS_ERR_PEVENT_NULL    If 'pevent' is a NULL pointer
*
* Note(s)    : As of V2.60, this function allows you to send NULL pointer messages.
*********************************************************************************************************
*/

#if OS_Q_POST_FRONT_EN > 0u
INT8U  OSQPostFront (OS_EVENT  *pevent,
                     void      *pmsg)
{
    rt_mq_t    pmq;
    rt_err_t   rt_err;
    ucos_msg_t ucos_msg;

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        return (OS_ERR_PEVENT_NULL);
    }
#endif

    pmq = (rt_mq_t)pevent->ipc_ptr;

    if (rt_object_get_type(&pmq->parent.parent)       /* Validate event block type                     */
        != RT_Object_Class_MessageQueue) {
        return (OS_ERR_EVENT_TYPE);
    }

    /*装填uCOS消息段*/
    ucos_msg.data_ptr = pmsg;
    rt_err = rt_mq_urgent(pmq, (void*)&ucos_msg, sizeof(ucos_msg_t));
    if(rt_err == -RT_EFULL) {
        return (OS_ERR_Q_FULL);
    }

    return (OS_ERR_NONE);
}
#endif


/*
*********************************************************************************************************
*                                       POST MESSAGE TO A QUEUE
*
* Description: This function sends a message to a queue.  This call has been added to reduce code size
*              since it can replace both OSQPost() and OSQPostFront().  Also, this function adds the
*              capability to broadcast a message to ALL tasks waiting on the message queue.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired queue
*
*              pmsg          is a pointer to the message to send.
*
*              opt           determines the type of POST performed:
*                            OS_POST_OPT_NONE         POST to a single waiting task
*                                                     (Identical to OSQPost())
*                            OS_POST_OPT_BROADCAST    POST to ALL tasks that are waiting on the queue
*                            OS_POST_OPT_FRONT        POST as LIFO (Simulates OSQPostFront())
*                          - OS_POST_OPT_NO_SCHED     Indicates that the scheduler will NOT be invoked
*
* Returns    : OS_ERR_NONE           The call was successful and the message was sent
*              OS_ERR_Q_FULL         If the queue cannot accept any more messages because it is full.
*              OS_ERR_EVENT_TYPE     If you didn't pass a pointer to a queue.
*              OS_ERR_PEVENT_NULL    If 'pevent' is a NULL pointer
*
* Warning    : Interrupts can be disabled for a long time if you do a 'broadcast'.  In fact, the
*              interrupt disable time is proportional to the number of tasks waiting on the queue.
*********************************************************************************************************
*/

#if OS_Q_POST_OPT_EN > 0u
INT8U  OSQPostOpt (OS_EVENT  *pevent,
                   void      *pmsg,
                   INT8U      opt)
{
    rt_mq_t    pmq;
    INT8U      err;
    ucos_msg_t ucos_msg;
    rt_err_t   rt_err;

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        return (OS_ERR_PEVENT_NULL);
    }
#endif

    pmq = (rt_mq_t)pevent->ipc_ptr;

    if (rt_object_get_type(&pmq->parent.parent)       /* Validate event block type                     */
        != RT_Object_Class_MessageQueue) {
        return (OS_ERR_EVENT_TYPE);
    }

    opt = opt & (OS_POST_OPT_NONE|OS_POST_OPT_BROADCAST|OS_POST_OPT_FRONT);
    if(opt == OS_POST_OPT_NONE) {
        err = OSQPost(pevent, pmsg);
    } else if(opt == OS_POST_OPT_BROADCAST) {
        ucos_msg.data_ptr = pmsg; /* 装填uCOS消息段 */
        rt_mq_send_all(pmq, (void*)&ucos_msg, sizeof(ucos_msg_t));
        if(rt_err == -RT_EFULL) {
            err = OS_ERR_Q_FULL;
        }
        err = OS_ERR_NONE;
    } else if(opt == OS_POST_OPT_FRONT) {
        err = OSQPostFront(pevent, pmsg);
    } else {
        err = OS_ERR_EVENT_TYPE;
    }

    return (err);
}
#endif


/*
*********************************************************************************************************
*                                        QUERY A MESSAGE QUEUE
*
* Description: This function obtains information about a message queue.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired queue
*
*              p_q_data      is a pointer to a structure that will contain information about the message
*                            queue.
*
* Returns    : OS_ERR_NONE         The call was successful and the message was sent
*              OS_ERR_EVENT_TYPE   If you are attempting to obtain data from a non queue.
*              OS_ERR_PEVENT_NULL  If 'pevent'   is a NULL pointer
*              OS_ERR_PDATA_NULL   If 'p_q_data' is a NULL pointer
*********************************************************************************************************
*/

#if OS_Q_QUERY_EN > 0u
INT8U  OSQQuery (OS_EVENT  *pevent,
                 OS_Q_DATA *p_q_data)
{
    rt_mq_t     pmq;
#if OS_CRITICAL_METHOD == 3u                           /* Allocate storage for CPU status register     */
    OS_CPU_SR   cpu_sr = 0u;
#endif

#if OS_ARG_CHK_EN > 0u
    if (pevent == (OS_EVENT *)0) {                     /* Validate 'pevent'                            */
        return (OS_ERR_PEVENT_NULL);
    }
    if (p_q_data == (OS_Q_DATA *)0) {                  /* Validate 'p_q_data'                          */
        return (OS_ERR_PDATA_NULL);
    }
#endif

    pmq = (rt_mq_t)pevent->ipc_ptr;

    if (rt_object_get_type(&pmq->parent.parent)       /* Validate event block type                     */
        != RT_Object_Class_MessageQueue) {
        return (OS_ERR_EVENT_TYPE);
    }

    OS_ENTER_CRITICAL();
    p_q_data->OSMsg = pmq->msg_queue_head;
    p_q_data->OSNMsgs = pmq->entry;
    p_q_data->OSQSize = pmq->max_msgs;
    OS_EXIT_CRITICAL();
    return (OS_ERR_NONE);
}
#endif                                                 /* OS_Q_QUERY_EN                                */

#endif                                                 /* OS_Q_EN                                      */
