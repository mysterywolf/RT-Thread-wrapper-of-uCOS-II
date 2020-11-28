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

#include "ucos_ii.h"

#if OS_TMR_EN > 0u

/*
*********************************************************************************************************
*                                          LOCAL PROTOTYPES
*********************************************************************************************************
*/
static void OS_TmrCallback(void *p_ara);

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

OS_TMR  *OSTmrCreate (INT32U           dly,
                      INT32U           period,
                      INT8U            opt,
                      OS_TMR_CALLBACK  callback,
                      void            *callback_arg,
                      INT8U           *pname,
                      INT8U           *perr)
{
    OS_TMR   *ptmr;
    rt_tick_t time, time2;
    rt_uint8_t rt_flag;
    
#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_TMR *)0);
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        *perr = OS_ERR_ILLEGAL_CREATE_RUN_TIME;
        return ((OS_TMR *)0);
    }
#endif
    
#if OS_ARG_CHK_EN > 0u
    switch (opt) {                                          /* Validate arguments                                     */
        case OS_TMR_OPT_PERIODIC:
             if (period == 0u) {
                 *perr = OS_ERR_TMR_INVALID_PERIOD;
                 return ((OS_TMR *)0);
             }
             break;

        case OS_TMR_OPT_ONE_SHOT:
             if (dly == 0u) {
                 *perr = OS_ERR_TMR_INVALID_DLY;
                 return ((OS_TMR *)0);
             }
             break;

        default:
             *perr = OS_ERR_TMR_INVALID_OPT;
             return ((OS_TMR *)0);
    }
#endif
    if (OSIntNesting > 0u) {                                /* See if trying to call from an ISR                      */
        *perr  = OS_ERR_TMR_ISR;
        return ((OS_TMR *)0);
    }    

    /*
     * uCOS-III原版定时器回调函数就是在定时器线程中调用的,而非在中断中调用,
     * 因此要使用RTT的RT_TIMER_FLAG_SOFT_TIMER选项,在此之前应将宏定义RT_USING_TIMER_SOFT置1
     */
    if(opt == OS_TMR_OPT_ONE_SHOT)
    {
        rt_flag = RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER;
        time = dly * (1000 / OS_TMR_CFG_TICKS_PER_SEC);     /* RTT和uCOS-III在定时器时钟源的设计不同,需要进行转换     */
    }
    else if(opt == OS_TMR_OPT_PERIODIC)
    {
        rt_flag = RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_SOFT_TIMER;     
        time = period * (1000 / OS_TMR_CFG_TICKS_PER_SEC);
    }
    
    ptmr = RT_KERNEL_MALLOC(sizeof(OS_TMR));                /* malloc OS_TMR                                          */
    if(!ptmr){
        *perr = OS_ERR_TMR_NON_AVAIL;
        return ((OS_TMR *)0);
    }
        
    OSSchedLock();
    ptmr->OSTmrState       = OS_TMR_STATE_STOPPED;          /* Indicate that timer is not running yet                 */
    ptmr->OSTmrDly         = dly;
    ptmr->OSTmrPeriod      = period;
    ptmr->_dly             = dly;
    ptmr->OSTmrOpt         = opt;
    ptmr->OSTmrCallback    = callback;
    ptmr->OSTmrCallbackArg = callback_arg;
    ptmr->OSTmrType        = OS_TMR_TYPE;
#if OS_TMR_CFG_NAME_EN > 0u
    if (pname == (INT8U *)0) {                              /* Is 'pname' a NULL pointer?                             */
        ptmr->OSTmrName    = (INT8U *)(void *)"?";
    } else {
        ptmr->OSTmrName    = pname;
    }
#endif  
    OSSchedUnlock();
    
    if(opt == OS_TMR_OPT_PERIODIC && dly != 0) {            /* 带有延迟的周期性延时                                   */
        time2 = dly * (1000 / OS_TMR_CFG_TICKS_PER_SEC);
        rt_timer_init(&ptmr->OSTmr, (const char*)pname,     /* invoke rt_timer_create to create a timer               */
            OS_TmrCallback, ptmr, time2, RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);
    }
    else {
        rt_timer_init(&ptmr->OSTmr, (const char*)pname,     /* invoke rt_timer_create to create a timer               */
            OS_TmrCallback, ptmr, time, rt_flag);        
    }
    
    *perr = OS_ERR_NONE;
    return (ptmr);   
}


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

BOOLEAN  OSTmrDel (OS_TMR  *ptmr,
                   INT8U   *perr)
{
#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (OS_FALSE);
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        *perr = OS_ERR_ILLEGAL_DEL_RUN_TIME;
        return (OS_FALSE);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (OS_FALSE);
    }
#endif
    
    if (ptmr->OSTmrType != OS_TMR_TYPE) {                   /* Validate timer structure                               */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (OS_FALSE);
    }
    if (OSIntNesting > 0u) {                                /* See if trying to call from an ISR                      */
        *perr  = OS_ERR_TMR_ISR;
        return (OS_FALSE);
    }
    if (ptmr->OSTmrState == OS_TMR_STATE_UNUSED){           /* Already deleted                                        */
        *perr = OS_ERR_TMR_INACTIVE;
        return (OS_FALSE);
    }

    OSSchedLock();
    ptmr->OSTmrState = OS_TMR_STATE_UNUSED;
    OSSchedUnlock();
    
    rt_timer_detach(&ptmr->OSTmr);                          /* 删除rt-thread定时器                                    */
    RT_KERNEL_FREE(ptmr);
    return (OS_TRUE);
}


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

#if OS_TMR_CFG_NAME_EN > 0u
INT8U  OSTmrNameGet (OS_TMR   *ptmr,
                     INT8U   **pdest,
                     INT8U    *perr)
{
#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pdest == (INT8U **)0) {
        *perr = OS_ERR_TMR_INVALID_DEST;
        return (0u);
    }
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (0u);
    }
#endif
    if (ptmr->OSTmrType != OS_TMR_TYPE) {              /* Validate timer structure                                    */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (0u);
    }
    if (OSIntNesting > 0u) {                           /* See if trying to call from an ISR                           */
        *perr = OS_ERR_NAME_GET_ISR;
        return (0u);
    }
    if (ptmr->OSTmrState == OS_TMR_STATE_UNUSED){      /* Already deleted                                             */
        *perr = OS_ERR_TMR_INACTIVE;
        return (0u);
    }
    
    *pdest = ptmr->OSTmrName;
    return OS_StrLen(*pdest);
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

INT32U  OSTmrRemainGet (OS_TMR  *ptmr,
                        INT8U   *perr)
{
    INT32U  remain;
    
#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (0u);
    }
#endif
    if (ptmr->OSTmrType != OS_TMR_TYPE) {              /* Validate timer structure                                    */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (0u);
    }
    if (OSIntNesting > 0u) {                           /* See if trying to call from an ISR                           */
        *perr = OS_ERR_TMR_ISR;
        return (0u);
    }
    OSSchedLock();
    switch (ptmr->OSTmrState) {
        case OS_TMR_STATE_RUNNING:
             ptmr->OSTmrMatch = ptmr->OSTmr.timeout_tick;
             remain = ptmr->OSTmrMatch - OSTmrTime;    /* Determine how much time is left to timeout                  */
             OSSchedUnlock();
             *perr  = OS_ERR_NONE;
             return (remain);

        case OS_TMR_STATE_STOPPED:                     /* It's assumed that the timer has not started yet             */
             switch (ptmr->OSTmrOpt) {
                 case OS_TMR_OPT_PERIODIC:
                      if (ptmr->OSTmrDly == 0u) {
                          remain = ptmr->OSTmrPeriod;
                      } else {
                          remain = ptmr->OSTmrDly;
                      }
                      OSSchedUnlock();
                      *perr  = OS_ERR_NONE;
                      break;

                 case OS_TMR_OPT_ONE_SHOT:
                 default:
                      remain = ptmr->OSTmrDly;
                      OSSchedUnlock();
                      *perr  = OS_ERR_NONE;
                      break;
             }
             return (remain);

        case OS_TMR_STATE_COMPLETED:                   /* Only ONE-SHOT that timed out can be in this state           */
             OSSchedUnlock();
             *perr = OS_ERR_NONE;
             return (0u);

        case OS_TMR_STATE_UNUSED:
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INACTIVE;
             return (0u);

        default:
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INVALID_STATE;
             return (0u);
    }
}


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

INT8U  OSTmrStateGet (OS_TMR  *ptmr,
                      INT8U   *perr)
{
    INT8U  state;


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (0u);
    }
#endif
    if (ptmr->OSTmrType != OS_TMR_TYPE) {              /* Validate timer structure                                    */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (0u);
    }
    if (OSIntNesting > 0u) {                           /* See if trying to call from an ISR                           */
        *perr = OS_ERR_TMR_ISR;
        return (0u);
    }
    OSSchedLock();
    state = ptmr->OSTmrState;
    switch (state) {
        case OS_TMR_STATE_UNUSED:
        case OS_TMR_STATE_STOPPED:
        case OS_TMR_STATE_COMPLETED:
        case OS_TMR_STATE_RUNNING:
             *perr = OS_ERR_NONE;
             break;

        default:
             *perr = OS_ERR_TMR_INVALID_STATE;
             break;
    }
    OSSchedUnlock();
    return (state);
}


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

BOOLEAN  OSTmrStart (OS_TMR   *ptmr,
                     INT8U    *perr)
{
#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (OS_FALSE);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (OS_FALSE);
    }
#endif
    
    if (ptmr->OSTmrType != OS_TMR_TYPE) {                   /* Validate timer structure                               */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (OS_FALSE);
    }
    if (OSIntNesting > 0u) {                                /* See if trying to call from an ISR                      */
        *perr  = OS_ERR_TMR_ISR;
        return (OS_FALSE);
    }   
    if(ptmr->OSTmrState == OS_TMR_STATE_UNUSED)
    {
        *perr = OS_ERR_TMR_INACTIVE;
        return (OS_FALSE);
    }
    
    rt_timer_start(&ptmr->OSTmr);
    OSSchedLock();
    ptmr->OSTmrMatch = ptmr->OSTmr.timeout_tick;
    OSSchedUnlock();
    *perr = OS_ERR_NONE;
    return (OS_TRUE);
}


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

BOOLEAN  OSTmrStop (OS_TMR  *ptmr,
                    INT8U    opt,
                    void    *callback_arg,
                    INT8U   *perr)
{
    OS_TMR_CALLBACK  pfnct;
    
#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (OS_FALSE);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (OS_FALSE);
    }
#endif

    if (ptmr->OSTmrType != OS_TMR_TYPE) {                         /* Validate timer structure                         */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (OS_FALSE);
    }
    if (OSIntNesting > 0u) {                                      /* See if trying to call from an ISR                */
        *perr  = OS_ERR_TMR_ISR;
        return (OS_FALSE);
    }

    OSSchedLock();
    switch (ptmr->OSTmrState) {
        case OS_TMR_STATE_RUNNING:
             *perr = OS_ERR_NONE;
             switch (opt) {
                 case OS_TMR_OPT_CALLBACK:
                      pfnct = ptmr->OSTmrCallback;                /* Execute callback function if available ...       */
                      if (pfnct != (OS_TMR_CALLBACK)0) {
                          (*pfnct)((void *)ptmr, ptmr->OSTmrCallbackArg);  /* Use callback arg when timer was created */
                      } else {
                          *perr = OS_ERR_TMR_NO_CALLBACK;
                      }
                      break;

                 case OS_TMR_OPT_CALLBACK_ARG:
                      pfnct = ptmr->OSTmrCallback;                /* Execute callback function if available ...       */
                      if (pfnct != (OS_TMR_CALLBACK)0) {
                          (*pfnct)((void *)ptmr, callback_arg);   /* ... using the 'callback_arg' provided in call    */
                      } else {
                          *perr = OS_ERR_TMR_NO_CALLBACK;
                      }
                      break;

                 case OS_TMR_OPT_NONE:
                      break;

                 default:
                     *perr = OS_ERR_TMR_INVALID_OPT;
                     break;
             }
             OSSchedUnlock();
             return (OS_TRUE);

        case OS_TMR_STATE_COMPLETED:                              /* Timer has already completed the ONE-SHOT or ...  */
        case OS_TMR_STATE_STOPPED:                                /* ... timer has not started yet.                   */
             OSSchedUnlock();
             *perr = OS_ERR_TMR_STOPPED;
             return (OS_TRUE);

        case OS_TMR_STATE_UNUSED:                                 /* Timer was not created                            */
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INACTIVE;
             return (OS_FALSE);

        default:
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INVALID_STATE;
             return (OS_FALSE);
    }
}


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

void  OSTmr_Init (void)
{
    /*nothing to do*/
}

/*
************************************************************************************************************************
*                                                   内部回调函数
*
* Description: 由于RT-Thread的定时器回调函数参数只有一个，而uCOS-III的定时器回调函数参数有两个，因此需要
*              先由RTT调用内部回调函数，再由内部回调函数调用uCOS-III的回调函数，以此完成参数的转换。
************************************************************************************************************************
*/
static void OS_TmrCallback(void *p_ara)
{
    OS_TMR   *ptmr;
    ptmr = (OS_TMR*)p_ara;
    
#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u    
    if(OSIntNesting > 0u)                                        /* 检查是否在中断中运行                    */
    {
        RT_DEBUG_LOG(OS_DEBUG_EN,("uCOS-II的定时器是在任务中运行的,不可以在RTT的Hard模式下运行\n"));
        return;
    }
#endif
    
    OSSchedLock(); 
    if(ptmr->OSTmrOpt == OS_TMR_OPT_PERIODIC && ptmr->_dly != 0)
    {
        ptmr->_dly = 0;
        ptmr->OSTmr.init_tick = ptmr->OSTmrPeriod * (1000 / OS_TMR_CFG_TICKS_PER_SEC);
        ptmr->OSTmr.timeout_tick = rt_tick_get() + ptmr->OSTmr.init_tick;
        ptmr->OSTmr.parent.flag |= RT_TIMER_FLAG_PERIODIC;       /* 定时器设置为周期模式                    */        
        ptmr->OSTmrMatch = rt_tick_get() + ptmr->OSTmr.init_tick;/* 重新设定下一次定时器的参数              */
        rt_timer_start(&(ptmr->OSTmr));                          /* 开启定时器                              */
    }
    else if(ptmr->OSTmrOpt == OS_TMR_OPT_ONE_SHOT)
    {
        ptmr->OSTmrState = OS_TMR_STATE_COMPLETED;
    }
    else if(ptmr->OSTmrOpt == OS_TMR_OPT_PERIODIC)
    {
        ptmr->OSTmrMatch = rt_tick_get() + ptmr->OSTmr.init_tick;/* 重新设定下一次定时器的参数              */
    }
    ptmr->OSTmrCallback(ptmr, ptmr->OSTmrCallbackArg);           /* 调用真正的uCOS-II定时器回调函数         */
    OSSchedUnlock();
}

#endif
