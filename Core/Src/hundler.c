/*
 * hundler.c
 *
 *  Created on: Oct 23, 2022
 *      Author: Evgeniy
 */

#include "windows.h"
#include "hundler.h"
#include "ws2812.h"
#include "stdio.h"
#include "main.h"
#include "st7789.h"
#include "arm_math.h"

extern SPI_HandleTypeDef hspi2;

extern TIM_HandleTypeDef htim2;	//триггер ацп

extern UART_HandleTypeDef huart1;

arm_rfft_fast_instance_f32 fft_struct;

uint8_t flag_adc_buf = 1;
int16_t adc_in_buf_1[FHT_LEN], adc_in_buf_2[FHT_LEN];
float32_t fft_in_buf[FHT_LEN];
float32_t fft_out_buf[FHT_LEN];

int16_t fx[FHT_LEN];
int16_t rx_buff, tx_buff = 0xAA01;

/************************************************************
 *				Инит функции ARM DSP FFT
 ***********************************************************/
void fft_init()
{
	arm_rfft_fast_init_f32(&fft_struct, FHT_LEN);
}

void SPI_DMAReceiveCplt(DMA_HandleTypeDef *hdma);

/************************************************************
 *		Запуск таймера, дергающий АЦП с нужным периодом
 ***********************************************************/
void timer_for_triggering_adc_init()
{
//	if (HAL_TIM_Base_Start_IT(&htim2) == HAL_OK)	//Запуск таймера для дискретизации
//		printf("TIM3_start Ok \r\n");
//	else printf("TIM3_start don't Ok \r\n");

	hspi2.hdmarx->XferCpltCallback = SPI_DMAReceiveCplt;

	HAL_DMA_Start_IT(hspi2.hdmarx, (uint32_t) &hspi2.Instance->DR, (uint32_t) &rx_buff, 1);
	SET_BIT(hspi2.Instance->CR2, SPI_CR2_RXDMAEN);
	__HAL_SPI_ENABLE(&hspi2);

	HAL_DMA_Start(htim2.hdma[TIM_DMA_ID_UPDATE], (uint32_t) &tx_buff, (uint32_t) &hspi2.Instance->DR, 1);
//	HAL_TIM_Base_Start_IT(&htim2);

	__HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
//	__HAL_TIM_MOE_ENABLE(&htim2);
//	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
//	__HAL_TIM_ENABLE(&htim2);
}

/***********************************************************
 *
 **********************************************************/
void SPI_DMAReceiveCplt(DMA_HandleTypeDef *hdma)
{
	if(hdma->Instance == DMA1_Stream3)
	{
		static uint16_t i;

		if (flag_adc_buf == 1)
		{
//			HAL_SPI_Receive(&hspi2, (uint8_t*) &buf, 1, 5000);
			fft_in_buf[i] = (float) (((rx_buff >> 1) & 0x1FFE) - 2047);
			i++;
			if (i == FHT_LEN)
			{
				HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
				i = 0;
				flag_adc_buf = 0;
				flag_data_processing = 1;
			}
		}
	}
}

/************************************************************
 *		Колбэк таймера с частотой дискретизации
 ***********************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
//	if (htim->Instance == TIM2)
//	{
//		HAL_GPIO_WritePin(ADC_NSS_GPIO_Port, ADC_NSS_Pin, RESET);
////		static uint16_t i;
////		int16_t buf;
////
////		if (flag_adc_buf == 1)
////		{
////			HAL_SPI_Receive(&hspi2, (uint8_t*) &buf, 1, 5000);
////			fft_in_buf[i] = (float) (((buf >> 1) & 0x1FFE) - 2047);
////			i++;
////			if (i == FHT_LEN)
////			{
////				i = 0;
////				flag_adc_buf = 0;
////				flag_data_processing = 1;
////			}
////		}
//	}
}

/************************************************************
 *
 ***********************************************************/
//float complexABS(float real, float compl)
//{
//	return sqrtf(real * real + compl * compl);
//}
/**************************************************************
 *			Семпл сохранен - колбек по завершению
 **************************************************************/
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
//{
//	if (hadc->Instance == ADC1)
//	{
//		HAL_GPIO_TogglePin(TEST_GPIO_Port, TEST_Pin);
//
//		if (flag_adc_buf == 1)
//		{
//			flag_adc_buf = 2;
//			HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_in_buf_2, FHT_LEN);
//			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);
//			for (uint16_t i = 0; i < FHT_LEN; ++i)
//			{
//				fft_in_buf[i] = adc_in_buf_1[i] - 2047;
//			}
//		}
//		else
//		{
//			flag_adc_buf = 1;
//			HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_in_buf_1, FHT_LEN);
//			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);
//			for (uint16_t i = 0; i < FHT_LEN; ++i)
//			{
//				fft_in_buf[i] = adc_in_buf_2[i] - 2047;
//			}
//		}
//	}
//	flag_data_processing = 1;
//}
/************************************************************
 *
 ***********************************************************/
void data_processing()
{
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
//			freqs[i] = (int) (fft_out_buf[i]) >> 12;
//			freqs[freqpoint] = (int) (20*log10f(complexABS(fft_out_buf[i], fft_out_buf[i + 1]))) - offset;
			if (freqs[i] < 0) freqs[i] = 0;
		}
		for (uint16_t i = 0; i < 160; ++i)
		{
			ST7789_DrawLine_for_Analyzer(i, freqs[i]);
		}
	}
	ST7789_SendFrame();

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);

	flag_data_processing = 0;
	flag_adc_buf = 1;
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}
