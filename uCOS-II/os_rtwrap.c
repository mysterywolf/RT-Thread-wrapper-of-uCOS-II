/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-07-12     Meco Man     the first verion
 */

#include "ucos_ii.h"
#include <string.h>
#include <rthw.h>

extern void (*rt_object_put_hook)(struct rt_object *object);

/**
 * 让当挂起表第一个任务放弃等待IPC,进入就绪态(由rt_ipc_list_resume函数改编)
 *
 * @param 挂起表表头指针
 *
 * @return 错误码
 */
rt_err_t rt_ipc_pend_abort_1 (rt_list_t *list)
{
    struct rt_thread *thread;
    register rt_ubase_t temp;
    OS_TCB  *p_tcb;

    temp = rt_hw_interrupt_disable();
    /* get thread entry */
    thread = rt_list_entry(list->next, struct rt_thread, tlist);
    p_tcb = (OS_TCB*)thread;
    /* set error code to RT_ERROR */
    thread->error = -RT_ERROR;

    /*标记当前任务放弃等待*/
    p_tcb->OSTCBStatPend = OS_STAT_PEND_ABORT;

    rt_hw_interrupt_enable(temp);

    /* resume it */
    rt_thread_resume(thread);

    return RT_EOK;
}

/**
 * 让所有等待该IPC的任务全部放弃等待，进入就绪态(由rt_ipc_list_resume_all函数改编)
 *
 * @param 挂起表表头指针
 *
 * @return 放弃了多少个任务
 */
rt_uint16_t rt_ipc_pend_abort_all (rt_list_t *list)
{
    struct rt_thread *thread;
    register rt_ubase_t temp;
    OS_TCB *p_tcb;
    rt_uint16_t i=0;

    /* wakeup all suspend threads */
    while (!rt_list_isempty(list))
    {
        /* disable interrupt */
        temp = rt_hw_interrupt_disable();

        /* get next suspend thread */
        thread = rt_list_entry(list->next, struct rt_thread, tlist);
        p_tcb = ((OS_TCB*)thread);
        /* set error code to RT_ERROR */
        thread->error = -RT_ERROR;

        /*标记当前任务放弃等待*/
        p_tcb->OSTCBStatPend = OS_STAT_PEND_ABORT;

        /*
         * resume thread
         * In rt_thread_resume function, it will remove current thread from
         * suspend list
         */
        rt_thread_resume(thread);

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        i++;
    }

    return i;
}

/**
 * 让所有等待该IPC的任务全部批准进入就绪态(由rt_ipc_list_resume_all函数改编)
 *
 * @param 挂起表表头指针
 *
 * @return 错误码
 */
static rt_err_t rt_ipc_post_all (rt_list_t *list)
{
    struct rt_thread *thread;
    register rt_ubase_t temp;

    /* wakeup all suspend threads */
    while (!rt_list_isempty(list))
    {
        /* disable interrupt */
        temp = rt_hw_interrupt_disable();

        /* get next suspend thread */
        thread = rt_list_entry(list->next, struct rt_thread, tlist);

        /*
         * resume thread
         * In rt_thread_resume function, it will remove current thread from
         * suspend list
         */
        rt_thread_resume(thread);

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);
    }

    return RT_EOK;
}

/**
 * This function will wake ALL threads which are WAITTING for message queue (FIFO)
 * 改编自rt_mq_send函数
 *
 * @param mq the message queue object
 * @param buffer the message
 * @param size the size of buffer
 *
 * @return the error code
 */
rt_err_t rt_mq_send_all(rt_mq_t mq, void *buffer, rt_size_t size)
{
    register rt_ubase_t temp;
    struct _rt_mq_message *msg;
    rt_uint16_t suspend_len;

    /* parameter check */
    RT_ASSERT(mq != RT_NULL);
    RT_ASSERT(rt_object_get_type(&mq->parent.parent) == RT_Object_Class_MessageQueue);
    RT_ASSERT(buffer != RT_NULL);
    RT_ASSERT(size != 0);

    /* greater than one message size */
    if (size > mq->msg_size)
        return -RT_ERROR;

    RT_OBJECT_HOOK_CALL(rt_object_put_hook, (&(mq->parent.parent)));

    /* disable interrupt */
    temp = rt_hw_interrupt_disable();

    /* 获取当前n个线程被当前消息队列挂起 */
    suspend_len = rt_list_len(&mq->parent.suspend_thread);

    /* 将相同的消息复制n次,一会一起发出去 */
    while(suspend_len)
    {
        /* get a free list, there must be an empty item */
        msg = (struct _rt_mq_message *)mq->msg_queue_free;
        /* message queue is full */
        if (msg == RT_NULL)
        {
            /* enable interrupt */
            rt_hw_interrupt_enable(temp);

            return -RT_EFULL;
        }
        /* move free list pointer */
        mq->msg_queue_free = msg->next;

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        /* the msg is the new tailer of list, the next shall be NULL */
        msg->next = RT_NULL;
        /* copy buffer */
        rt_memcpy(msg + 1, buffer, size);

        /* disable interrupt */
        temp = rt_hw_interrupt_disable();
        /* link msg to message queue */
        if (mq->msg_queue_tail != RT_NULL)
        {
            /* if the tail exists, */
            ((struct _rt_mq_message *)mq->msg_queue_tail)->next = msg;
        }

        /* set new tail */
        mq->msg_queue_tail = msg;
        /* if the head is empty, set head */
        if (mq->msg_queue_head == RT_NULL)
            mq->msg_queue_head = msg;

        /* increase message entry */
        mq->entry ++;

        suspend_len --;
    }

    /* resume suspended thread */
    if (!rt_list_isempty(&mq->parent.suspend_thread))
    {
        /* 将等待本消息队列的所有线程全部解挂,此时他们将同时获得相同的消息 */
        rt_ipc_post_all(&(mq->parent.suspend_thread));

        /* enable interrupt */
        rt_hw_interrupt_enable(temp);

        rt_schedule();

        return RT_EOK;
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(temp);

    return RT_EOK;
}


/**
 * 自动初始化
 * uCOS-II兼容层支持按照uCOS-II原版的初始化步骤进行初始化，但是在有些情况，
 * 用户不想手动初始化uCOS-II兼容层，想要直接运行应用层任务或模块，则可以使用该
 * 宏定义。在rtconfig.h中定义本宏定义后，在RT-Thread初始化完成并进入到main线程之前
 * 会自动将uCOS-II兼容层初始化完毕，用户仅需要专注于uCOS-II的应用级任务即可。
 * The wrapper supports uCOS-II standard startup procedure. Alternatively,
 * if you want to run uCOS-II apps directly and ignore the startup procedure,
 * you can choose this option.
 */
#ifdef PKG_USING_UCOSII_WRAPPER_AUTOINIT
static int rt_ucosii_autoinit(void)
{
    OSInit();                                       /*uCOS-II操作系统初始化*/
    OSStart();                                      /*开始运行uCOS-II操作系统*/

#if OS_TASK_STAT_EN > 0u
    OSStatInit();
#endif
    return 0;
}
INIT_PREV_EXPORT(rt_ucosii_autoinit);
#endif
