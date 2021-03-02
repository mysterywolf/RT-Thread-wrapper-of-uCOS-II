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
 * �õ�������һ����������ȴ�IPC,�������̬(��rt_ipc_list_resume�����ı�)
 *
 * @param ������ͷָ��
 *
 * @return ������
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

    /*��ǵ�ǰ��������ȴ�*/
    p_tcb->OSTCBStatPend = OS_STAT_PEND_ABORT;

    rt_hw_interrupt_enable(temp);

    /* resume it */
    rt_thread_resume(thread);

    return RT_EOK;
}

/**
 * �����еȴ���IPC������ȫ�������ȴ����������̬(��rt_ipc_list_resume_all�����ı�)
 *
 * @param ������ͷָ��
 *
 * @return �����˶��ٸ�����
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

        /*��ǵ�ǰ��������ȴ�*/
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
 * �����еȴ���IPC������ȫ����׼�������̬(��rt_ipc_list_resume_all�����ı�)
 *
 * @param ������ͷָ��
 *
 * @return ������
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
 * �ı���rt_mq_send����
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

    /* ��ȡ��ǰn���̱߳���ǰ��Ϣ���й��� */
    suspend_len = rt_list_len(&mq->parent.suspend_thread);

    /* ����ͬ����Ϣ����n��,һ��һ�𷢳�ȥ */
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
        /* ���ȴ�����Ϣ���е������߳�ȫ�����,��ʱ���ǽ�ͬʱ�����ͬ����Ϣ */
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
 * �Զ���ʼ��
 * uCOS-II���ݲ�֧�ְ���uCOS-IIԭ��ĳ�ʼ��������г�ʼ������������Щ�����
 * �û������ֶ���ʼ��uCOS-II���ݲ㣬��Ҫֱ������Ӧ�ò������ģ�飬�����ʹ�ø�
 * �궨�塣��rtconfig.h�ж��屾�궨�����RT-Thread��ʼ����ɲ����뵽main�߳�֮ǰ
 * ���Զ���uCOS-II���ݲ��ʼ����ϣ��û�����Ҫרע��uCOS-II��Ӧ�ü����񼴿ɡ�
 * The wrapper supports uCOS-II standard startup procedure. Alternatively,
 * if you want to run uCOS-II apps directly and ignore the startup procedure,
 * you can choose this option.
 */
#ifdef PKG_USING_UCOSII_WRAPPER_AUTOINIT
static int rt_ucosii_autoinit(void)
{
    OSInit();                                       /*uCOS-II����ϵͳ��ʼ��*/
    OSStart();                                      /*��ʼ����uCOS-II����ϵͳ*/

#if OS_TASK_STAT_EN > 0u
    OSStatInit();
#endif
    return 0;
}
INIT_PREV_EXPORT(rt_ucosii_autoinit);
#endif
