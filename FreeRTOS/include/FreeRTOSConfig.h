/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

// 1 - вытесняющая, 0 - кооперативная
#define configUSE_PREEMPTION		1
#define configUSE_IDLE_HOOK			1
#define configUSE_TICK_HOOK			0
#define configCPU_CLOCK_HZ			( ( unsigned long ) 100000000 )
#define configTICK_RATE_HZ			( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES		( 5 )
#define configMINIMAL_STACK_SIZE	( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE		( ( size_t ) ( 17 * 1024 ) )
#define configMAX_TASK_NAME_LEN		( 16 )

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS            1
#define configUSE_TRACE_FACILITY                 1
#define configUSE_STATS_FORMATTING_FUNCTIONS     1

#define configUSE_16_BIT_TICKS		0
#define configIDLE_SHOULD_YIELD		1

#define configUSE_COUNTING_SEMAPHORES			1


/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete				1
#define INCLUDE_vTaskCleanUpResources	0
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay				1

/* This is the raw value as per the Cortex-M3 NVIC.  Values can be 255
(lowest) to 0 (1?) (highest). */
#define configKERNEL_INTERRUPT_PRIORITY 		255
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	191 /* equivalent to 0xb0, or priority 11. */


/* This is the value being used as per the ST library which permits 16
priority values, 0 to 15.  This must correspond to the
configKERNEL_INTERRUPT_PRIORITY setting.  Here 15 corresponds to the lowest
NVIC value of 255. */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY	15

volatile unsigned long ulHighFrequencyTimerTicks;

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() ( ulHighFrequencyTimerTicks = 0UL )
#define portGET_RUN_TIME_COUNTER_VALUE() ulHighFrequencyTimerTicks

#define xPortSysTickHandler 		SysTick_Handler
#define xPortPendSVHandler 			PendSV_Handler
#define vPortSVCHandler 			SVC_Handler

#endif /* FREERTOS_CONFIG_H */


///*
// * FreeRTOS V202212.01
// * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
// *
// * Permission is hereby granted, free of charge, to any person obtaining a copy of
// * this software and associated documentation files (the "Software"), to deal in
// * the Software without restriction, including without limitation the rights to
// * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// * the Software, and to permit persons to whom the Software is furnished to do so,
// * subject to the following conditions:
// *
// * The above copyright notice and this permission notice shall be included in all
// * copies or substantial portions of the Software.
// *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// *
// * https://www.FreeRTOS.org
// * https://github.com/FreeRTOS
// *
// */
//
//#ifndef FREERTOS_CONFIG_H
//#define FREERTOS_CONFIG_H
//
///*-----------------------------------------------------------
// * Application specific definitions.
// *
// * These definitions should be adjusted for your particular hardware and
// * application requirements.
// *
// * These parameters and more are described within the 'configuration' section of the
// * FreeRTOS API documentation available on the FreeRTOS.org web site.
// *
// * See http://www.freertos.org/a00110.html
// *----------------------------------------------------------*/
//
//#define configENABLE_FPU                         0
//#define configENABLE_MPU                         0
//
//#define configUSE_PREEMPTION                     1
//#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0
//#define configUSE_TICKLESS_IDLE                  0
//#define configCPU_CLOCK_HZ                       ( ( unsigned long ) 100000000 )
//#define configTICK_RATE_HZ                       ((TickType_t)1000)
//#define configMAX_PRIORITIES                     ( 5 )
//#define configMINIMAL_STACK_SIZE                 ((uint16_t)128)
//#define configMAX_TASK_NAME_LEN                  ( 16 )
//#define configUSE_16_BIT_TICKS                   0
//#define configIDLE_SHOULD_YIELD                  1
//#define configUSE_TASK_NOTIFICATIONS             1
//#define configTASK_NOTIFICATION_ARRAY_ENTRIES    3
//#define configUSE_MUTEXES                        0
//#define configUSE_RECURSIVE_MUTEXES              0
//#define configUSE_COUNTING_SEMAPHORES            0
//#define configUSE_ALTERNATIVE_API                0 /* Deprecated! */
//#define configQUEUE_REGISTRY_SIZE                10
//#define configUSE_QUEUE_SETS                     0
//#define configUSE_TIME_SLICING                   0
//#define configUSE_NEWLIB_REENTRANT               0
//#define configENABLE_BACKWARD_COMPATIBILITY      1
//#define configNUM_THREAD_LOCAL_STORAGE_POINTERS  5
//#define configUSE_MINI_LIST_ITEM                 1
//#define configSTACK_DEPTH_TYPE                   uint16_t
//#define configMESSAGE_BUFFER_LENGTH_TYPE         size_t
//#define configHEAP_CLEAR_MEMORY_ON_FREE          1
//
///* Memory allocation related definitions. */
//#define configSUPPORT_STATIC_ALLOCATION          	0
//#define configSUPPORT_DYNAMIC_ALLOCATION         	1
//#define configTOTAL_HEAP_SIZE                       ((size_t)15360)
//#define configAPPLICATION_ALLOCATED_HEAP            1
//#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP   1
//
///* Hook function related definitions. */
//#define configUSE_IDLE_HOOK                      0
//#define configUSE_TICK_HOOK                      0
//#define configCHECK_FOR_STACK_OVERFLOW           0
//#define configUSE_MALLOC_FAILED_HOOK             0
//#define configUSE_DAEMON_TASK_STARTUP_HOOK       0
//#define configUSE_SB_COMPLETED_CALLBACK          0
//
///* Run time and task stats gathering related definitions. */
//#define configGENERATE_RUN_TIME_STATS            0
//#define configUSE_TRACE_FACILITY                 0
//#define configUSE_STATS_FORMATTING_FUNCTIONS     0
//
///* Co-routine definitions. */
//#define configUSE_CO_ROUTINES                    0
//#define configMAX_CO_ROUTINE_PRIORITIES          2
//
///* Software timer definitions. */
//#define configUSE_TIMERS                         1
//#define configTIMER_TASK_PRIORITY                2
//#define configTIMER_QUEUE_LENGTH                 10
//#define configTIMER_TASK_STACK_DEPTH             configMINIMAL_STACK_SIZE
//
///* Cortex-M specific definitions. */
//#ifdef __NVIC_PRIO_BITS
// /* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
// #define configPRIO_BITS         __NVIC_PRIO_BITS
//#else
//#define configPRIO_BITS         4
//#endif
//
///* The lowest interrupt priority that can be used in a call to a "set priority"
// function. */
//#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15
//
///* The highest interrupt priority that can be used by any interrupt service
// routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
// INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
// PRIORITY THAN THIS! (higher priorities are lower numeric values. */
//#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
//
///* Interrupt priorities used by the kernel port layer itself.  These are generic
// to all Cortex-M ports, and do not rely on any particular library functions. */
//#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
///* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
// See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
//#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
//
///* Normal assert() semantics without relying on the provision of an assert.h
// header file. */
///* USER CODE BEGIN 1 */
//#define configASSERT( x ) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );}
///* USER CODE END 1 */
//
///* Set the following definitions to 1 to include the API function, or zero
// to exclude the API function. */
//#define INCLUDE_vTaskPrioritySet             1
//#define INCLUDE_uxTaskPriorityGet            1
//#define INCLUDE_vTaskDelete                  1
//#define INCLUDE_vTaskCleanUpResources        0
//#define INCLUDE_vTaskSuspend                 1
//#define INCLUDE_vTaskDelayUntil              1
//#define INCLUDE_vTaskDelay                   1
//#define INCLUDE_xTaskGetSchedulerState       1
//#define INCLUDE_xTimerPendFunctionCall       1
//#define INCLUDE_xQueueGetMutexHolder         1
//#define INCLUDE_uxTaskGetStackHighWaterMark  1
//#define INCLUDE_xTaskGetCurrentTaskHandle    1
//#define INCLUDE_eTaskGetState                1
//
///* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
// standard names. */
////#define xPortSysTickHandler 		SysTick_Handler
//#define vPortSVCHandler    			SVC_Handler
//#define xPortPendSVHandler 			PendSV_Handler
//
//#endif /* FREERTOS_CONFIG_H */
