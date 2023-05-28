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



extern ADC_HandleTypeDef hadc1;
//DMA_HandleTypeDef hdma_adc1;

extern TIM_HandleTypeDef htim2;	//триггер ацп
extern TIM_HandleTypeDef htim3;	//ШИМ светодиоды
//DMA_HandleTypeDef hdma_tim3_ch2;

extern UART_HandleTypeDef huart1;

arm_rfft_fast_instance_f32 fft_struct;

uint8_t flag_adc_buf;
int16_t adc_in_buf_1[FHT_LEN], adc_in_buf_2[FHT_LEN];
float32_t fft_in_buf[FHT_LEN];
float32_t fft_out_buf[FHT_LEN];

int16_t fx[FHT_LEN];

/************************************************************
 *				Инит функции ARM DSP FFT
 ***********************************************************/
void fft_init()
{
	arm_rfft_fast_init_f32(&fft_struct, FHT_LEN);
}

/************************************************************
 *		Запуск таймера, дергающий АЦП с нужным периодом
 ***********************************************************/
void timer_for_triggering_adc_init()
{
	if (HAL_TIM_Base_Start(&htim2) == HAL_OK)	//Запуск таймера для дискретизации
	printf("TIM3_start Ok \r\n");
	else printf("TIM3_start don't Ok \r\n");

	if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_in_buf_1, FHT_LEN) == HAL_OK)	//Запуск ДМА от АЦП в память
	{
		flag_adc_buf = 1;
		printf("HAL_ADC_start Ok \r\n");
	}
	else printf("HAL_ADC_start don't Ok \r\n");
}

#ifdef WS2812
/************************************************************
 *			Таймер закончил отправку на ленту
 ***********************************************************/
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3)
	{
		HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_1);
		TIM3->CCR1 = 0;
		TIM3->CNT = 0;

		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_in_buf, FHT_LEN);
	}
}
#endif

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
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
	{
		HAL_GPIO_TogglePin(TEST_GPIO_Port, TEST_Pin);

		if (flag_adc_buf == 1)
		{
			flag_adc_buf = 2;
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_in_buf_2, FHT_LEN);
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);
			for (uint16_t i = 0; i < FHT_LEN; ++i)
			{
				fft_in_buf[i] = adc_in_buf_1[i] - 2047;
			}
		}
		else
		{
			flag_adc_buf = 1;
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_in_buf_1, FHT_LEN);
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);
			for (uint16_t i = 0; i < FHT_LEN; ++i)
			{
				fft_in_buf[i] = adc_in_buf_2[i] - 2047;
			}
		}
	}
	flag_data_processing = 1;
}

/************************************************************
 *
 ***********************************************************/
void data_processing()
{
	if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == 0)
	{
		for (uint16_t i = 0; i < ST7789_HEIGHT; ++i)
		{
			ST7789_DrawLine_for_Analyzer(i, fft_in_buf[i] / 32);
		}
	}
	else
	{
		int freqs[FHT_LEN / 2];
		int offset = 0;	//variable noisefloor offset

		applyHammingWindowFloat(fft_in_buf);
		arm_rfft_fast_f32(&fft_struct, (float32_t*) &fft_in_buf, (float32_t*) &fft_out_buf, 0);
		arm_cmplx_mag_f32(fft_out_buf, fft_out_buf, FHT_LEN);

		for (int i = 0; i < FHT_LEN / 2; ++i)
		{
//			freqs[i] = (int) (20 * (log10f(fft_out_buf[i]))) - offset;
			freqs[i] = (int) (fft_out_buf[i]) >> 12;
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

#ifdef WS2812
	static uint8_t amplitude[25];

	//uint8_t harmonic[25] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,25,29,35,42,49};

	for (uint8_t column = 0; column < WIDTH; ++column)
	{
		int temp;
		//temp = (float)fft_in_buf[column+1]/3.15f;

		temp = freqs[column + 1] / 11;

		if ((uint8_t) temp >= amplitude[column]) amplitude[column] = (uint8_t) temp;  //эффект падающего столбца
		else amplitude[column]--;

		if (column % 2)	//для нечетных столбцов инверсное заполнение шкал
		{
			for (uint8_t k = 0; k < (AMPL - amplitude[column]); ++k)	//Очищает остаток шкалы
			{
				ws2812_rgb_to_buf(0, 0, 0, column * AMPL + k);
			}
			for (uint8_t k = (AMPL - amplitude[column]); k < AMPL; ++k)	//Заполняет шкалу цветом
			{
				ws2812_rgb_to_buf(0, 0, 20, column * AMPL + k);
			}
		}
		else
		{
			for (uint8_t k = 0; k < amplitude[column]; ++k)
			{
				ws2812_rgb_to_buf(0, 0, 20, column * AMPL + k);
			}
			for (uint8_t k = amplitude[column]; k < AMPL; ++k)
			{
				ws2812_rgb_to_buf(0, 0, 0, column * AMPL + k);
			}
		}
	}
	ws2812_start();
#else

#endif
}
