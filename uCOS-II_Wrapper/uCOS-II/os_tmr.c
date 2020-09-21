/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-21     Meco Man     first version
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
*                                           TIMER MANAGEMENT
*
* Filename : os_tmr.c
* Version  : V2.93.00
*********************************************************************************************************
*/

#ifndef  OS_TMR_C
#define  OS_TMR_C

#ifndef  OS_MASTER_FILE
#include <ucos_ii.h>
#endif

/*
*********************************************************************************************************
*                                                        NOTES
*
* 1) Your application MUST define the following #define constants:
*
*    OS_TASK_TMR_PRIO          The priority of the Timer management task
*    OS_TASK_TMR_STK_SIZE      The size     of the Timer management task's stack
*
* 2) You must call OSTmrSignal() to notify the Timer management task that it's time to update the timers.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define  OS_TMR_LINK_DLY       0u
#define  OS_TMR_LINK_PERIODIC  1u

/*
*********************************************************************************************************
*                                          LOCAL PROTOTYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           CREATE A TIMER
*
* Description: This function is called by your application code to create a timer.
*
* Arguments  : dly           Initial delay.
*                            If the timer is configured for ONE-SHOT mode, this is the timeout used.
*                            If the timer is configured for PERIODIC mode, this is the first timeout to
*                               wait for before the timer starts entering periodic mode.
*
*              period        The 'period' being repeated for the timer.
*                               If you specified 'OS_TMR_OPT_PERIODIC' as an option, when the timer
*                               expires, it will automatically restart with the same period.
*
*              opt           Specifies either:
*                               OS_TMR_OPT_ONE_SHOT       The timer counts down only once
*                               OS_TMR_OPT_PERIODIC       The timer counts down and then reloads itself
*
*              callback      Is a pointer to a callback function that will be called when the timer expires.
*                               The callback function must be declared as follows:
*
*                               void MyCallback (OS_TMR *ptmr, void *p_arg);
*
*              callback_arg  Is an argument (a pointer) that is passed to the callback function when it is called.
*
*              pname         Is a pointer to an ASCII string that is used to name the timer.  Names are
*                               useful for debugging.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE                     the call was successful and the timer
*                                                               was created.
*                               OS_ERR_ILLEGAL_CREATE_RUN_TIME  if you tried to create a timer after
*                                                               safety critical operation started.
*                               OS_ERR_TMR_INVALID_DLY          you specified an invalid delay
*                               OS_ERR_TMR_INVALID_PERIOD       you specified an invalid period
*                               OS_ERR_TMR_INVALID_OPT          you specified an invalid option
*                               OS_ERR_TMR_ISR                  if the call was made from an ISR
*                               OS_ERR_TMR_NON_AVAIL            if there are no free timers from the timer pool
*
* Returns    : A pointer to an OS_TMR data structure.
*              This is the 'handle' that your application will use to reference the timer created.
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
OS_TMR  *OSTmrCreate (INT32U           dly,
                      INT32U           period,
                      INT8U            opt,
                      OS_TMR_CALLBACK  callback,
                      void            *callback_arg,
                      INT8U           *pname,
                      INT8U           *perr)
{

}
#endif


/*
*********************************************************************************************************
*                                           DELETE A TIMER
*
* Description: This function is called by your application code to delete a timer.
*
* Arguments  : ptmr          Is a pointer to the timer to stop and delete.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE                  the call was successful and the timer
*                                                            was deleted.
*                               OS_ERR_ILLEGAL_DEL_RUN_TIME  if you tried to delete a timer after safety
*                                                            critical operation started.
*                               OS_ERR_TMR_INVALID           'ptmr'  is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE      'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR               if the function was called from an ISR
*                               OS_ERR_TMR_INACTIVE          if the timer was not created
*                               OS_ERR_TMR_INVALID_STATE     the timer is in an invalid state
*
* Returns    : OS_TRUE       If the call was successful
*              OS_FALSE      If not
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
BOOLEAN  OSTmrDel (OS_TMR  *ptmr,
                   INT8U   *perr)
{

}
#endif


/*
*********************************************************************************************************
*                                       GET THE NAME OF A TIMER
*
* Description: This function is called to obtain the name of a timer.
*
* Arguments  : ptmr          Is a pointer to the timer to obtain the name for
*
*              pdest         Is a pointer to pointer to where the name of the timer will be placed.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE               The call was successful
*                               OS_ERR_TMR_INVALID_DEST   'pdest' is a NULL pointer
*                               OS_ERR_TMR_INVALID        'ptmr'  is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_NAME_GET_ISR       if the call was made from an ISR
*                               OS_ERR_TMR_INACTIVE       'ptmr'  points to a timer that is not active
*                               OS_ERR_TMR_INVALID_STATE  the timer is in an invalid state
*
* Returns    : The length of the string or 0 if the timer does not exist.
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u && OS_TMR_CFG_NAME_EN > 0u
INT8U  OSTmrNameGet (OS_TMR   *ptmr,
                     INT8U   **pdest,
                     INT8U    *perr)
{

}
#endif


/*
*********************************************************************************************************
*                          GET HOW MUCH TIME IS LEFT BEFORE A TIMER EXPIRES
*
* Description: This function is called to get the number of ticks before a timer times out.
*
* Arguments  : ptmr          Is a pointer to the timer to obtain the remaining time from.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID        'ptmr' is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR            if the call was made from an ISR
*                               OS_ERR_TMR_INACTIVE       'ptmr' points to a timer that is not active
*                               OS_ERR_TMR_INVALID_STATE  the timer is in an invalid state
*
* Returns    : The time remaining for the timer to expire.  The time represents 'timer' increments.
*              In other words, if OSTmr_Task() is signaled every 1/10 of a second then the returned
*              value represents the number of 1/10 of a second remaining before the timer expires.
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
INT32U  OSTmrRemainGet (OS_TMR  *ptmr,
                        INT8U   *perr)
{

}
#endif


/*
*********************************************************************************************************
*                                  FIND OUT WHAT STATE A TIMER IS IN
*
* Description: This function is called to determine what state the timer is in:
*
*                  OS_TMR_STATE_UNUSED     the timer has not been created
*                  OS_TMR_STATE_STOPPED    the timer has been created but has not been started or has been stopped
*                  OS_TMR_STATE_COMPLETED  the timer is in ONE-SHOT mode and has completed it's timeout
*                  OS_TMR_STATE_RUNNING    the timer is currently running
*
* Arguments  : ptmr          Is a pointer to the desired timer
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID        'ptmr' is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR            if the call was made from an ISR
*                               OS_ERR_TMR_INACTIVE       'ptmr' points to a timer that is not active
*                               OS_ERR_TMR_INVALID_STATE  if the timer is not in a valid state
*
* Returns    : The current state of the timer (see description).
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
INT8U  OSTmrStateGet (OS_TMR  *ptmr,
                      INT8U   *perr)
{

}
#endif


/*
*********************************************************************************************************
*                                            START A TIMER
*
* Description: This function is called by your application code to start a timer.
*
* Arguments  : ptmr          Is a pointer to an OS_TMR
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID
*                               OS_ERR_TMR_INVALID_TYPE    'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR             if the call was made from an ISR
*                               OS_ERR_TMR_INACTIVE        if the timer was not created
*                               OS_ERR_TMR_INVALID_STATE   the timer is in an invalid state
*
* Returns    : OS_TRUE    if the timer was started
*              OS_FALSE   if an error was detected
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
BOOLEAN  OSTmrStart (OS_TMR   *ptmr,
                     INT8U    *perr)
{

}
#endif


/*
*********************************************************************************************************
*                                            STOP A TIMER
*
* Description: This function is called by your application code to stop a timer.
*
* Arguments  : ptmr          Is a pointer to the timer to stop.
*
*              opt           Allows you to specify an option to this functions which can be:
*
*                               OS_TMR_OPT_NONE          Do nothing special but stop the timer
*                               OS_TMR_OPT_CALLBACK      Execute the callback function, pass it the
*                                                        callback argument specified when the timer
*                                                        was created.
*                               OS_TMR_OPT_CALLBACK_ARG  Execute the callback function, pass it the
*                                                        callback argument specified in THIS function call.
*
*              callback_arg  Is a pointer to a 'new' callback argument that can be passed to the callback
*                            function instead of the timer's callback argument.  In other words, use
*                            'callback_arg' passed in THIS function INSTEAD of ptmr->OSTmrCallbackArg.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID         'ptmr' is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE    'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR             if the function was called from an ISR
*                               OS_ERR_TMR_INACTIVE        if the timer was not created
*                               OS_ERR_TMR_INVALID_OPT     if you specified an invalid option for 'opt'
*                               OS_ERR_TMR_STOPPED         if the timer was already stopped
*                               OS_ERR_TMR_INVALID_STATE   the timer is in an invalid state
*                               OS_ERR_TMR_NO_CALLBACK     if the timer does not have a callback function defined
*
* Returns    : OS_TRUE       If we stopped the timer (if the timer is already stopped, we also return OS_TRUE)
*              OS_FALSE      If not
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
BOOLEAN  OSTmrStop (OS_TMR  *ptmr,
                    INT8U    opt,
                    void    *callback_arg,
                    INT8U   *perr)
{

}
#endif


/*
*********************************************************************************************************
*                             SIGNAL THAT IT'S TIME TO UPDATE THE TIMERS
*
* Description: This function is typically called by the ISR that occurs at the timer tick rate and is
*              used to signal to OSTmr_Task() that it's time to update the timers.
*
* Arguments  : none
*
* Returns    : OS_ERR_NONE         The call was successful and the timer task was signaled.
*              OS_ERR_SEM_OVF      If OSTmrSignal() was called more often than OSTmr_Task() can handle
*                                  the timers. This would indicate that your system is heavily loaded.
*              OS_ERR_EVENT_TYPE   Unlikely you would get this error because the semaphore used for
*                                  signaling is created by uC/OS-II.
*              OS_ERR_PEVENT_NULL  Again, unlikely you would ever get this error because the semaphore
*                                  used for signaling is created by uC/OS-II.
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
INT8U  OSTmrSignal (void)
{

}
#endif


/*
*********************************************************************************************************
*                                                    INITIALIZATION
*                                          INITIALIZE THE FREE LIST OF TIMERS
*
* Description: This function is called by OSInit() to initialize the free list of OS_TMRs.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
void  OSTmr_Init (void)
{

}
#endif

#endif                                                           /* OS_TMR_C                                */
