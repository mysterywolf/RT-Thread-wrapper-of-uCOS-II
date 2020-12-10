/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-17     Meco Man     first version
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
*                                 uC/OS-II Configuration File for V2.9x
*
* Filename : os_cfg.h
* Version  : V2.93.00
*********************************************************************************************************
*/

#ifndef OS_CFG_H
#define OS_CFG_H

#include <rtconfig.h>
                                       /* ---------------------- MISCELLANEOUS ----------------------- */
#if defined RT_USING_HOOK && defined RT_USING_IDLE_HOOK
#define OS_APP_HOOKS_EN           1u   /* 读写 Application-defined hooks are called from the uC/OS-II hooks */
#define OS_CPU_HOOKS_EN           1u   /* 读写 uC/OS-II hooks are found in the processor port files    */
#else
#define OS_APP_HOOKS_EN           0u   /* 只读 Application-defined hooks are called from the uC/OS-II hooks */
#define OS_CPU_HOOKS_EN           0u   /* 只读 uC/OS-II hooks are found in the processor port files    */
#endif

#define OS_ARG_CHK_EN             1u   /* Enable (1) or Disable (0) argument checking                  */

#define OS_DEBUG_EN               0u   /* Enable(1) debug variables                                    */

#define OS_EVENT_MULTI_EN         0u   /* 只读 Include code for OSEventPendMulti()                     */
#define OS_EVENT_NAME_EN          1u   /* Enable names for Sem, Mutex, Mbox and Q                      */

#define OS_LOWEST_PRIO    RT_THREAD_PRIORITY_MAX /* 只读 Defines the lowest priority that can be assigned...*/
                                       /* ... MUST NEVER be higher than 254!                           */

#define OS_MAX_MEM_PART           5u   /* Max. number of memory partitions                             */
#define OS_MAX_TASKS             20u   /* Max. number of tasks in your application, MUST be >= 2       */

#define OS_SCHED_LOCK_EN          1u   /* Include code for OSSchedLock() and OSSchedUnlock()           */

#define OS_TICKS_PER_SEC  RT_TICK_PER_SECOND /* 只读 Set the number of ticks in one second             */

#define OS_TLS_TBL_SIZE           0u   /* Size of Thread-Local Storage Table                           */


                                       /* --------------------- TASK STACK SIZE ---------------------- */
#define OS_TASK_STAT_STK_SIZE   128u   /* Statistics task stack size (# of OS_STK wide entries)        */

                                       /* --------------------- TASK MANAGEMENT ---------------------- */
#define OS_TASK_CHANGE_PRIO_EN    1u   /*     Include code for OSTaskChangePrio()                      */
#define OS_TASK_CREATE_EN         1u   /*     Include code for OSTaskCreate()                          */
#define OS_TASK_CREATE_EXT_EN     1u   /*     Include code for OSTaskCreateExt()                       */
#define OS_TASK_DEL_EN            1u   /*     Include code for OSTaskDel()                             */
#define OS_TASK_NAME_EN           1u   /*     Enable task names                                        */
#define OS_TASK_PROFILE_EN        1u   /*     Include variables in OS_TCB for profiling                */
#define OS_TASK_QUERY_EN          1u   /*     Include code for OSTaskQuery()                           */
#define OS_TASK_REG_TBL_SIZE      1u   /*     Size of task variables array (#of INT32U entries)        */
#define OS_TASK_STAT_EN           1u   /*     Enable (1) or Disable(0) the statistics task             */
#define OS_TASK_STAT_STK_CHK_EN   1u   /*     Check task stacks from statistic task                    */
#define OS_TASK_SUSPEND_EN        1u   /*     Include code for OSTaskSuspend() and OSTaskResume()      */
#define OS_TASK_SW_HOOK_EN        1u   /*     Include code for OSTaskSwHook()                          */


                                       /* ----------------------- EVENT FLAGS ------------------------ */
#ifdef RT_USING_EVENT                  /* 是否开启由RT-Thread接管                                      */
#define OS_FLAG_EN                1u   /* 读写 Enable (1) or Disable (0) code generation for EVENT FLAGS*/
#else
#define OS_FLAG_EN                0u   /* 只读 Enable (1) or Disable (0) code generation for EVENT FLAGS*/
#endif
#define OS_FLAG_ACCEPT_EN         0u   /*     Include code for OSFlagAccept()                          */
#define OS_FLAG_DEL_EN            0u   /*     Include code for OSFlagDel()                             */
#define OS_FLAG_QUERY_EN          0u   /*     Include code for OSFlagQuery()                           */
#define OS_FLAG_WAIT_CLR_EN       0u   /* Include code for Wait on Clear EVENT FLAGS                   */

                                       /* --------------------- MEMORY MANAGEMENT -------------------- */
#define OS_MEM_EN                 1u   /* Enable (1) or Disable (0) code generation for MEMORY MANAGER */
#define OS_MEM_NAME_EN            1u   /*     Enable memory partition names                            */
#define OS_MEM_QUERY_EN           1u   /*     Include code for OSMemQuery()                            */


                                       /* ---------------- MUTUAL EXCLUSION SEMAPHORES --------------- */
#ifdef RT_USING_MUTEX                  /* 是否开启由RT-Thread接管                                      */
#define OS_MUTEX_EN               1u   /* 读写 Enable (1) or Disable (0) code generation for MUTEX     */
#else
#define OS_MUTEX_EN               0u   /* 只读 Enable (1) or Disable (0) code generation for MUTEX     */
#endif
#define OS_MUTEX_ACCEPT_EN        1u   /*     Include code for OSMutexAccept()                         */
#define OS_MUTEX_DEL_EN           1u   /*     Include code for OSMutexDel()                            */
#define OS_MUTEX_QUERY_EN         1u   /*     Include code for OSMutexQuery()                          */


                                       /* ---------------------- MESSAGE QUEUES ---------------------- */
#ifdef RT_USING_MESSAGEQUEUE           /* 是否开启由RT-Thread接管                                      */
#define OS_Q_EN                   1u   /* 读写 Enable (1) or Disable (0) code generation for QUEUES    */
#else
#define OS_Q_EN                   0u   /* 只读 Enable (1) or Disable (0) code generation for QUEUES    */
#endif
#define OS_Q_ACCEPT_EN            1u   /*     Include code for OSQAccept()                             */
#define OS_Q_DEL_EN               1u   /*     Include code for OSQDel()                                */
#define OS_Q_FLUSH_EN             1u   /*     Include code for OSQFlush()                              */
#define OS_Q_PEND_ABORT_EN        1u   /*     Include code for OSQPendAbort()                          */
#define OS_Q_POST_EN              1u   /*     Include code for OSQPost()                               */
#define OS_Q_POST_FRONT_EN        1u   /*     Include code for OSQPostFront()                          */
#define OS_Q_POST_OPT_EN          1u   /*     Include code for OSQPostOpt()                            */
#define OS_Q_QUERY_EN             1u   /*     Include code for OSQQuery()                              */


                                       /* -------------------- MESSAGE MAILBOXES --------------------- */
#define OS_MBOX_EN           OS_Q_EN   /* 只读 Enable (1) or Disable (0) code generation for MAILBOXES */
#define OS_MBOX_ACCEPT_EN         1u   /*     Include code for OSMboxAccept()                          */
#define OS_MBOX_DEL_EN            1u   /*     Include code for OSMboxDel()                             */
#define OS_MBOX_PEND_ABORT_EN     1u   /*     Include code for OSMboxPendAbort()                       */
#define OS_MBOX_POST_EN           1u   /*     Include code for OSMboxPost()                            */
#define OS_MBOX_POST_OPT_EN       1u   /*     Include code for OSMboxPostOpt()                         */
#define OS_MBOX_QUERY_EN          1u   /*     Include code for OSMboxQuery()                           */


                                       /* ------------------------ SEMAPHORES ------------------------ */
#ifdef RT_USING_SEMAPHORE              /* 是否开启由RT-Thread接管                                      */
#define OS_SEM_EN                 1u   /* 读写 Enable (1) or Disable (0) code generation for SEMAPHORES*/
#else
#define OS_SEM_EN                 0u   /* 只读 Enable (1) or Disable (0) code generation for SEMAPHORES*/
#endif
#define OS_SEM_ACCEPT_EN          1u   /*    Include code for OSSemAccept()                            */
#define OS_SEM_DEL_EN             1u   /*    Include code for OSSemDel()                               */
#define OS_SEM_PEND_ABORT_EN      1u   /*    Include code for OSSemPendAbort()                         */
#define OS_SEM_QUERY_EN           1u   /*    Include code for OSSemQuery()                             */
#define OS_SEM_SET_EN             1u   /*    Include code for OSSemSet()                               */


                                       /* --------------------- TIME MANAGEMENT ---------------------- */
#define OS_TIME_DLY_HMSM_EN       1u   /*     Include code for OSTimeDlyHMSM()                         */
#define OS_TIME_DLY_RESUME_EN     1u   /*     Include code for OSTimeDlyResume()                       */
#define OS_TIME_GET_SET_EN        1u   /*     Include code for OSTimeGet() and OSTimeSet()             */


                                       /* --------------------- TIMER MANAGEMENT --------------------- */
#ifdef RT_USING_TIMER_SOFT             /* 是否开启由RT-Thread接管                                      */
#define OS_TMR_EN                 1u   /* 读写 Enable (1) or Disable (0) code generation for TIMERS    */
#else
#define OS_TMR_EN                 0u   /* 只读 Enable (1) or Disable (0) code generation for TIMERS    */
#endif
#define OS_TMR_CFG_NAME_EN        1u   /*     Determine timer names                                    */
#define OS_TMR_CFG_TICKS_PER_SEC 10u   /*     Rate at which timer management task runs (Hz)            */

#endif
