/*
 * Copyright (c) 2021, Meco Jianting Man <jiantingman@foxmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-18     Meco Man     first version
 */
/*
;********************************************************************************************************
;                                              uC/OS-II
;                                        The Real-Time Kernel
;
;                    Copyright 1992-2020 Silicon Laboratories Inc. www.silabs.com
;
;                                 SPDX-License-Identifier: APACHE-2.0
;
;               This software is subject to an open source license and is distributed by
;                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
;                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
;
;********************************************************************************************************
*/

#include "os_cpu.h"
#include <rthw.h>

/*
;********************************************************************************************************
;                                          PUBLIC FUNCTIONS
;********************************************************************************************************
*/

/*
;********************************************************************************************************
;                                   CRITICAL SECTION METHOD 3 FUNCTIONS
;
; Description : Disable/Enable Kernel aware interrupts by preserving the state of BASEPRI.  Generally speaking,
;               the state of the BASEPRI interrupt exception processing is stored in the local variable
;               'cpu_sr' & Kernel Aware interrupts are then disabled ('cpu_sr' is allocated in all functions
;               that need to disable Kernel aware interrupts). The previous BASEPRI interrupt state is restored
;               by copying 'cpu_sr' into the BASEPRI register.
;
; Prototypes  : OS_CPU_SR  OS_CPU_SR_Save   (OS_CPU_SR  new_basepri);
;               void       OS_CPU_SR_Restore(OS_CPU_SR  cpu_sr);
;
;
; Note(s)     : 1) These functions are used in general like this:
;
;                  void Task (void *p_arg)
;                  {
;                  #if OS_CRITICAL_METHOD == 3          // Allocate storage for CPU status register
;                      OS_CPU_SR  cpu_sr;
;                  #endif
;
;                          :
;                          :
;                      OS_ENTER_CRITICAL();             // cpu_sr = OS_CPU_SR_Save(new_basepri);
;                          :
;                          :
;                      OS_EXIT_CRITICAL();              // OS_CPU_RestoreSR(cpu_sr);
;                          :
;                          :
;                  }
;
;               2) Increasing priority using a write to BASEPRI does not take effect immediately.
;                  (a) IMPLICATION  This erratum means that the instruction after an MSR to boost BASEPRI
;                      might incorrectly be preempted by an insufficient high priority exception.
;
;                  (b) WORKAROUND  The MSR to boost BASEPRI can be replaced by the following code sequence:
;
;                      CPSID i
;                      MSR to BASEPRI
;                      DSB
;                      ISB
;                      CPSIE i
;********************************************************************************************************
*/

#if OS_CRITICAL_METHOD == 3
OS_CPU_SR OS_CPU_SR_Save (void)
{
    return rt_hw_interrupt_disable();
}

void OS_CPU_SR_Restore (OS_CPU_SR cpu_sr)
{
    rt_hw_interrupt_enable(cpu_sr);
}
#endif
