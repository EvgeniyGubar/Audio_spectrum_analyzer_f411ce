/*
 * hundler.c
 *
 *  Created on: Oct 23, 2022
 *      Author: Evgeniy
 */

#include "hundler.h"

#include "fhtConfig.h"
#include "sampling.h"
#include "arm_math.h"

extern ADC_HandleTypeDef hadc1;
//DMA_HandleTypeDef hdma_adc1;

extern TIM_HandleTypeDef htim2;	//триггер ацп
extern TIM_HandleTypeDef htim3;	//ШИМ светодиоды
//DMA_HandleTypeDef hdma_tim3_ch2;

arm_rfft_fast_instance_f32 fft_struct;

int16_t adc_in_buf[FHT_LEN];
float32_t fft_in_buf[FHT_LEN];
float32_t fft_out_buf[FHT_LEN];



/************************************************************
 * Таймер закончил отправку на ленту
 ***********************************************************/
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
	{
		HAL_TIM_PWM_Stop_DMA(&htim3,TIM_CHANNEL_2);
		TIM3->CCR1 = 0;
		TIM3->CNT = 0;

		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_in_buf, FHT_LEN);
#ifdef DEBUGGING_PIN
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);  //начало семпла
#endif
	}
}

/**********************************************************/

float complexABS(float real, float compl)
{
	return sqrtf(real*real+compl*compl);
}

/**
 * 	Семпл сохранен - колбек по завершению
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance == ADC1)
	{
#ifdef DEBUGGING_PIN
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET); //конец семпла
#endif

		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);

		for (uint16_t i = 0; i < FHT_LEN; ++i)
		{
			fft_in_buf[i] = (float)((adc_in_buf[i]<<3) - 16383);  //делаем двуполярный сигнал и масштабируем до +/- 16383
		}


		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);



		arm_rfft_fast_f32(&fft_struct, (float32_t*)&fft_in_buf, (float32_t*)&fft_out_buf, 0);

		int freqs[FHT_LEN/2];
		int freqpoint = 0;
		int offset = 150;	//variable noisefloor offset

		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);
		//calculate abs values and linear-to-dB
		for (int i=0; i<FHT_LEN; i=i+2)
		{
			freqs[freqpoint] = (int)(20*log10f(complexABS(fft_out_buf[i], fft_out_buf[i+1])))-offset;
			if (freqs[freqpoint]<0) freqs[freqpoint]=0;
			freqpoint++;
		}


		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, RESET);

//		applyHannWindow(fft_in_buf);
//		fhtDitInt(fft_in_buf);
//		complexToDecibel(fx);

		static uint8_t amplitude[25];

		//uint8_t harmonic[25] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,25,29,35,42,49};

		for (uint8_t column = 0; column < WIDTH; ++column)
		{
			int temp;
			//temp = (float)fft_in_buf[column+1]/3.15f;

			temp = freqs[column+1]/11;


 			if ( (uint8_t)temp >= amplitude[column] ) amplitude[column] = (uint8_t)temp;  //эффект падающего столбца
			else amplitude[column]--;


			if (column % 2)	//для нечетных столбцов инверсное заполнение шкал
			{
				for (uint8_t k = 0; k < (AMPL - amplitude[column]); ++k)	//Очищает остаток шкалы
				{
					ws2812_rgb_to_buf(0, 0, 0, column*AMPL + k);
				}
				for (uint8_t k = (AMPL - amplitude[column]); k < AMPL ; ++k)	//Заполняет шкалу цветом
				{
					ws2812_rgb_to_buf(0, 0, 20, column*AMPL + k);
				}
			}
			else
			{
				for (uint8_t k = 0; k < amplitude[column]; ++k)
				{
					ws2812_rgb_to_buf(0, 0, 20, column*AMPL + k);
				}
				for (uint8_t k = amplitude[column]; k < AMPL; ++k)
				{
					ws2812_rgb_to_buf(0, 0, 0, column*AMPL + k);
				}
			}
		}

		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, SET);
		ws2812_start();
	}
}

/**************************************************************/
void adc_init()
{

//	if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_in_buf, FHT_LEN) == HAL_OK)	//Запуск ДМА от АЦП в память
//		printf("HAL_ADC_start Ok \r\n");
//	else
//		printf("HAL_ADC_start don't Ok \r\n");

	arm_rfft_fast_init_f32(&fft_struct, 2048);
}

/**************************************************************/
void timer_for_triggering_adc_init()
{
	if(HAL_TIM_Base_Start_IT(&htim2) == HAL_OK)	//Запуск таймера для дискретизации
		printf("TIM3_start Ok \r\n");
	else
		printf("TIM3_start don't Ok \r\n");
}

/**************************************************************/
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	printf("Error conversion \r\n");
}
