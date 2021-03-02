/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-17     Meco Man     first version
 */

/*���ļ�չʾ�˼��ݲ�ʵ�ֵ�uCOS-II�ٷ������ı�׼��ʼ������*/
/*�����Լ���ʼ�������ϸ����չٷ�����������*/
/*����ʹ��USART2*/

#include <ucos_ii.h>

void task_example (void);
void timer_example (void);
void sem_example (void);
void mutex_example (void);
void messagequeue_example (void);
void flag_example (void);

int main(void)/*RT-Thread main�߳�*/
{
    OSInit();
    OSStart();

#if OS_TASK_STAT_EN > 0u
    OSStatInit();
#endif

//    task_example();
//    timer_example();
//    sem_example();
//    mutex_example();
//    messagequeue_example();
    flag_example();
}
