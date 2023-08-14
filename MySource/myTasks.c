///*
// * myTasks.c
// *
// *  Created on: 13 авг. 2023 г.
// *      Author: Evgeniy
// */
//
////#include "myTasks.h"
////#include "main.h"
////#include "FreeRTOS.h"
////#include "task.h"
////#include "queue.h"
////#include "semphr.h"
////#include "hundler.h"
////#include "arm_math.h"
////#include "st7789.h"
////#include "windows.h"
////
////extern arm_rfft_fast_instance_f32 fft_struct;
////extern float32_t fft_in_buf[FHT_LEN];
////extern float32_t fft_out_buf[FHT_LEN];
////
////xQueueHandle MyQueue;
////xSemaphoreHandle Semaph_data_ready;
////xSemaphoreHandle MySemaphoreCounting;
////
////void task_led_1(void const *argument);
////void task_led_2(void const *argument);
////void BUTTON_Task(void const *argument);
////void Task_New(void const *argument);
////void task_data_processing(void const *argument);
////
////void createRTOS()
////{
//////	MyQueue = xQueueCreate(3, sizeof(uint8_t));
////	vSemaphoreCreateBinary(Semaph_data_ready);
//////	MySemaphoreCounting = xSemaphoreCreateCounting(5, 0);
////
//////	if (MyQueue != NULL)
//////	{
////	if (Semaph_data_ready != NULL)
////	{
////		xTaskCreate((void*) task_led_1, "Led 1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
////		xTaskCreate((void*) task_data_processing, "Led 2", configMINIMAL_STACK_SIZE * 100, NULL, 2, NULL);
//////		xTaskCreate((void*) BUTTON_Task, "Task3", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
//////		xTaskCreate((void*) Task_New, "Task3", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
////
////		vTaskStartScheduler();
////	}
//////	}
////}
////
/////************************************************************
//// *
//// ***********************************************************/
////void task_data_processing(void const *argument)
////{
////	for (;;)
////	{
////		if (xSemaphoreTake(Semaph_data_ready, portMAX_DELAY) == pdTRUE)
////		{
////
////			if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0)
////			{
////				for (uint16_t i = 0; i < ST7789_HEIGHT; ++i)
////				{
////					ST7789_DrawLine_for_Analyzer(i, (fft_in_buf[i] + 2047) / 32);
////				}
////			}
////			else
////			{
////				int freqs[FHT_LEN / 2];
////				int offset = 33;	//variable noisefloor offset
////
////				applyHammingWindowFloat(fft_in_buf);
////				arm_rfft_fast_f32(&fft_struct, (float32_t*) &fft_in_buf, (float32_t*) &fft_out_buf, 0);
////				arm_cmplx_mag_f32(fft_out_buf, fft_out_buf, FHT_LEN);
////
////				for (int i = 0; i < FHT_LEN / 2; ++i)
////				{
////					freqs[i] = (int) (20 * (log10f(fft_out_buf[i]))) - offset;
////					if (freqs[i] < 0) freqs[i] = 0;
////				}
////				for (uint16_t i = 0; i < 160; ++i)
////				{
////					ST7789_DrawLine_for_Analyzer(i, freqs[i]);
////				}
////			}
////			ST7789_SendFrame();
////		}
////	}
////	vTaskDelete(NULL);
//////	fft_data_ready = 0;
////}
//
///******************************************************************************/
//void Task_New(void const *argument)
//{
//	portTickType xLastWakeTime;
//	xLastWakeTime = xTaskGetTickCount();
//
//	for (;;)
//	{
//		xTaskDelayUntil(&xLastWakeTime, 50);
//	}
//	vTaskDelete(NULL);
//}
//
///******************************************************************************/
//void task_led_1(void const *argument)
//{
////	portTickType xLastWakeTime;
////	xLastWakeTime = xTaskGetTickCount();
//	for (;;)
//	{
////		/* Реализовано ожидание события с помощью двоичного
////		 семафора. Семафор после создания становится
////		 доступен (так, как будто его кто-то отдал).
////		 Поэтому сразу после запуска планировщика задача
////		 захватит его. Второй раз сделать это ей не удастся,
////		 и она будет ожидать, находясь в блокированном
////		 состоянии, пока семафор не отдаст обработчик
////		 прерывания. Время ожидания задано равным
////		 бесконечности, поэтому нет необходимости проверять
////		 возвращаемое функцией xSemaphoreTake() значение. */
////		xSemaphoreTake(MySemaphoreCounting, portMAX_DELAY);
////
////		/* Если программа “дошла” до этого места, значит,
////		 семафор был успешно захвачен.
////		 Обработка события, связанного с семафором.
////		 В нашем случае – моргание светодиода*/
//		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//
////		xTaskDelayUntil(&xLastWakeTime, 200);
//		vTaskDelay(500);
//	}
//	vTaskDelete(NULL);
//}

///******************************************************************************
// * Демонстрация работы с очередью */
//void task_led_2(void const *argument)
//{
//	for (;;)
//	{
//////		Status = xQueueReceive(MyQueue, &counter, portMAX_DELAY);
////		if (Status == pdPASS)
////		{
//////			HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, SET);
////			for (uint8_t i = counter; i > 0; i--)
////			{
//////				vTaskDelay(50);
////				for (uint32_t i = 0; i < 500000; i++);
////			}
//////			HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, RESET);
////			vTaskDelay(50);
////		}
//	}
//	vTaskDelete(NULL);
//}
///******************************************************************************
// * Демонстрация работы с очередью */
//void BUTTON_Task(void const *argument)
//{
//	for (;;)
//	{
////		if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0)
////		{
////			counter = 0;
////			while (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0)
////			{
////				counter++;
////				for (uint32_t i = 0; i < 500000; i++);
////
//////				vTaskDelay(50);
////			}
////			Status = xQueueSendToBack(MyQueue, &counter, 100);
////			if (Status != pdPASS)
////			{
////				//
////			}
////		}
//	}
//	vTaskDelete(NULL);
//}
