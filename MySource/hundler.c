/*
 * hundler.c
 *
 *  Created on: Oct 23, 2022
 *      Author: Evgeniy
 */
#include "hundler.h"
#include "main.h"
#include "windows.h"
#include "st7789.h"
#include "stdio.h"
#include "arm_math.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;
extern xSemaphoreHandle Semaph_data_ready;

arm_rfft_fast_instance_f32 fft_struct;
float32_t fft_in_buf[FHT_LEN];
float32_t fft_out_buf[FHT_LEN];

float32_t adc_buf[FHT_LEN / 2];
float32_t fft_in_buf_1[FHT_LEN], fft_in_buf_2[FHT_LEN];

int16_t rx_buff, tx_buff; //tx_buff фиктивная переменная для запуска работы SPI на прием

xQueueHandle MyQueue;
xSemaphoreHandle Semaph_data_ready;
xSemaphoreHandle MySemaphoreCounting;

void SPI_DMAReceiveCplt_from_ADC(DMA_HandleTypeDef *hdma);
void led1Task(void const *argument);
void debugTask(void const *argument);
void BUTTON_Task(void const *argument);
void Task_New(void const *argument);
void processingTask(void const *argument);

void createRTOS()
{
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

//	MyQueue = xQueueCreate(3, sizeof(uint8_t));
	vSemaphoreCreateBinary(Semaph_data_ready);
//	MySemaphoreCounting = xSemaphoreCreateCounting(5, 0);

	if (Semaph_data_ready != NULL)
	{
		xTaskCreate((void*) led1Task, "Led 1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		xTaskCreate((void*) debugTask, "debugTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		xReturned = xTaskCreate((void*) processingTask, "Led 2", configMINIMAL_STACK_SIZE * 10, NULL, 1, &xHandle);
		if (xReturned != pdPASS)
		{
			Error_Handler();
		}

		vTaskStartScheduler();
	}
}
/******************************************************************************/
void led1Task(void const *argument)
{
	for (;;)
	{
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		vTaskDelay(500);
	}
	vTaskDelete(NULL);
}

void debugTask(void const *argument)
{
	for (;;)
	{
		vTaskDelay(500);
	}
	vTaskDelete(NULL);
}

/************************************************************
 *
 ***********************************************************/
void processingTask(void const *argument)
{
	for (;;)
	{
		/* Блокирующий семафор, ожидает готового семпла*/
		xSemaphoreTake(Semaph_data_ready, portMAX_DELAY);

		if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0)
		{
			for (uint16_t i = 0; i < ST7789_HEIGHT; ++i)
			{
				ST7789_DrawLine_for_Analyzer(i, (fft_in_buf[i] + 2047) / 32);
			}
		}
		else
		{
			int freqs[FHT_LEN / 2];
			int offset = 33;	//variable noisefloor offset

			applyHammingWindowFloat(fft_in_buf);
			arm_rfft_fast_f32(&fft_struct, (float32_t*) &fft_in_buf, (float32_t*) &fft_out_buf, 0);
			arm_cmplx_mag_f32(fft_out_buf, fft_out_buf, FHT_LEN);

			for (int i = 0; i < FHT_LEN / 2; ++i)
			{
				freqs[i] = (int) (20 * (log10f(fft_out_buf[i]))) - offset;
				if (freqs[i] < 0) freqs[i] = 0;
			}
			for (uint16_t i = 0; i < 160; ++i)
			{
				ST7789_DrawLine_for_Analyzer(i, freqs[i]);
			}
		}
		ST7789_SendFrame();
	}
	vTaskDelete(NULL);
}

/************************************************************
 *				Инит функции ARM DSP FFT
 ***********************************************************/
void fft_init()
{
	arm_rfft_fast_init_f32(&fft_struct, FHT_LEN);
}

/************************************************************
 *    Конфигурирование каналов DMA, запуск SPI и TIM
 ***********************************************************/
void fft_hardware_init()
{
	/* Указатель на функцию обратного вызова по окончанию работы DMA */
	hspi2.hdmarx->XferCpltCallback = SPI_DMAReceiveCplt_from_ADC;
	/* Настройка DMA на передачу от SPI2 в память по приему данных */
	HAL_DMA_Start_IT(hspi2.hdmarx, (uint32_t) &hspi2.Instance->DR, (uint32_t) &rx_buff, 1);
	/* Установка бита разрешения дергать DMA по принятию данных в регистр SPI_DR*/
	SET_BIT(hspi2.Instance->CR2, SPI_CR2_RXDMAEN);
	/* Разрешить работы SPI2 */
	__HAL_SPI_ENABLE(&hspi2);

	/* Настройка DMA: дергает таймер, DMA отправляет из памяти в SPI фиктивные данные на отправку*/
	HAL_DMA_Start(htim2.hdma[TIM_DMA_ID_UPDATE], (uint32_t) &tx_buff, (uint32_t) &hspi2.Instance->DR, 1);
	__HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);
	/* Запуск таймера в режиме PWM для генерации на канале 1 сигнала NSS для SPI*/
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

/**********************************************************/
void SPI_DMAReceiveCplt_from_ADC(DMA_HandleTypeDef *hdma)
{
	if (hdma->Instance == DMA1_Stream3)
	{
		static uint16_t i;
		static uint8_t State = 0;

		/* Подготовка данных*/
		adc_buf[i] = (float) (((rx_buff & 0x1FFE) >> 1) - 2047);

		i++;
		if (i == FHT_LEN / 2)
		{
			i = 0;

			switch (State) {
			case 0:
			{
				State = 1;
				arm_copy_f32(adc_buf, &fft_in_buf_1[FHT_LEN / 2], FHT_LEN / 2);
				arm_copy_f32(adc_buf, fft_in_buf_2, FHT_LEN / 2);
				arm_copy_f32(fft_in_buf_1, fft_in_buf, FHT_LEN);
				break;
			}
			case 1:
			{
				State = 2;
				arm_copy_f32(adc_buf, fft_in_buf_1, FHT_LEN / 2);
				arm_copy_f32(adc_buf, &fft_in_buf_2[FHT_LEN / 2], FHT_LEN / 2);
				arm_copy_f32(fft_in_buf_2, fft_in_buf, FHT_LEN);
				break;
			}
			case 2:
			{
				State = 3;
				arm_copy_f32(adc_buf, &fft_in_buf_1[FHT_LEN / 2], FHT_LEN / 2);
				arm_copy_f32(adc_buf, fft_in_buf_2, FHT_LEN / 2);
				arm_copy_f32(fft_in_buf_1, fft_in_buf, FHT_LEN);
				break;
			}
			case 3:
			{
				State = 0;
				arm_copy_f32(adc_buf, fft_in_buf_1, FHT_LEN / 2);
				arm_copy_f32(adc_buf, &fft_in_buf_2[FHT_LEN / 2], FHT_LEN / 2);
				arm_copy_f32(fft_in_buf_2, fft_in_buf, FHT_LEN);
				break;
			}
			default:
				break;
			}


			static portBASE_TYPE xHigherPriorityTaskWoken;
			xHigherPriorityTaskWoken = pdFALSE;
			/* Отдать семафор задаче-обработчику */
			xSemaphoreGiveFromISR(Semaph_data_ready, &xHigherPriorityTaskWoken);
			if (xHigherPriorityTaskWoken != pdFALSE)
			{
				portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
			}
		}
	}
}
