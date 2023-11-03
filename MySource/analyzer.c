/*
 * hundler.c
 *
 *  Created on: Oct 23, 2022
 *      Author: Evgeniy
 */
#include "MicroMenu.h"
#include "analyzer.h"
#include "main.h"
#include "windows.h"
#include "st7789.h"
#include "lcd_analyzer.h"
#include "stdio.h"
#include "arm_math.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define DEBUG_TASK	0

extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim11;
extern ADC_HandleTypeDef hadc1;
extern I2S_HandleTypeDef hi2s3;

arm_rfft_fast_instance_f32 fft_struct;

int16_t rx_buff, tx_buff;				//tx_buff фиктивная переменная для запуска работы SPI на прием
float32_t adc_buf[FHT_LEN / 2];
uint16_t raw_adc_data[FHT_LEN * 2];		// двойной запас на 2 канала при работе с микрофоном
float32_t fft_in_buf_1[FHT_LEN], fft_in_buf_2[FHT_LEN];
float32_t fft_in_buf[FHT_LEN];
float32_t fft_out_buf[FHT_LEN];

xSemaphoreHandle Semaph_data_ready;
xSemaphoreHandle Semaph_CPU_load;
xQueueHandle Queue_encoder;
xQueueHandle Queue_menu_navigate;

void externalADC_cmpl_callback(DMA_HandleTypeDef *hdma);
void led1Task(void const *argument);
void encoderTask(void const *argument);
#if (DEBUG_TASK == 1)
void debugTask(void const *argument);
#endif
void fftTask(void const *argument);
void menuTask(void const *argument);
void buttEncTask(void const *argument);

void internalADC_Init();
void internalADC_Deinit();
void externalADC_Init();
void externalADC_Deinit();
void microphone_Init();
void microphone_Deinit();

typedef enum {
	BUTTON_NONE,
	BUTTON_PREVIOUS,
	BUTTON_NEXT,
	BUTTON_PARENT,
	BUTTON_CHILD,
	BUTTON_ENTER
} ButtonValues;

typedef enum {
	MODE_ANALYZER,
	MODE_SCOPE,
	MODE_AUDIO_SPECTR
} Mode;

typedef enum {
	INPUT_EXT_ADC,
	INPUT_INT_ADC,
	INPUT_MICRO
} Input;

/* Settings parameters */
Mode globalMode = MODE_ANALYZER;
Input globalInput = INPUT_MICRO;
int8_t globalNoise = 0;
float globalScale = 1;

/* Debug variables*/
#if (DEBUG_TASK == 1)
volatile static uint8_t CPU_IDLE = 0;
char pcWriteBuffer[1024];
char loadWriteBuffer[1024];
uint32_t freemem;
#endif

char textBuff[30];

#if (DEBUG_TASK == 1)
/***********************************************************/
uint8_t GetCPU_IDLE()
{
	return CPU_IDLE;
}
#endif
/***********************************************************/
void vApplicationIdleHook()
{
#if (DEBUG_TASK == 1)
	static portTickType LastTick = 0;
	static uint32_t count;			//наш трудяга счетчик
	static uint32_t max_count;		//максимальное значение счетчика, вычисляется при калибровке
									//и соответствует 100% CPU idle
	count++;						//приращение счетчика

	if (xTaskGetTickCount() - LastTick > 400)				//если прошло 400 тиков
	{
		LastTick = xTaskGetTickCount();
		if (count > max_count) max_count = count;         	//это калибровка
		CPU_IDLE = 100 * (max_count - count) / max_count;	//вычисляем текущую загрузку
		count = 0;               							//обнуляем счетчик
		xSemaphoreGive(Semaph_CPU_load);
	}
#endif
}
/***********************************************************/
static void constrain(int8_t *parameter, int8_t min, int8_t max)
{
	if (*parameter < min)
	{
		*parameter = max;
		return;
	}
	if (*parameter > max)
	{
		*parameter = min;
		return;
	}
	return;
}
/***********************************************************/
static int8_t readEncoder()
{
	ButtonValues Value = BUTTON_NONE;
	portBASE_TYPE xStatus;

	xStatus = xQueueReceive(Queue_menu_navigate, &Value, 100);
	if (xStatus == pdPASS)
	{
		if (Value == BUTTON_NEXT) return 1;
		if (Value == BUTTON_PREVIOUS) return -1;
	}
	return 0;
}
/***********************************************************/
static void modeMain(void)
{
	switch (globalMode)
	{
	case MODE_ANALYZER:
		{
			switch (globalInput)
			{
			case INPUT_MICRO:
				{
					ST7789_DrawGraphNet_Mic();
				}
				break;
			default:
				ST7789_DrawGraphNet_ADC();
			}
			sprintf(textBuff, "Analyzer  ");
		}
		break;
	case MODE_SCOPE:
		ST7789_DrawGraphClean();
		sprintf(textBuff, "Scope     ");
		break;
	case MODE_AUDIO_SPECTR:
		ST7789_DrawGraphClean();
		sprintf(textBuff, "Audio     ");
		break;
	}
	ST7789_DrawText_7x11(81, 1, WHITE, textBuff, Right, updateVRAM, BLACK);
}
/***********************************************************/
static void noiseMain(void)
{
	snprintf(textBuff, sizeof(textBuff), "%d         ", globalNoise);
	ST7789_DrawText_7x11(81, 1, WHITE, textBuff, Right, updateVRAM, BLACK);
}
/***********************************************************/
static void scaleMain(void)
{
	snprintf(textBuff, sizeof(textBuff), "%0.1f       ", globalScale);
	ST7789_DrawText_7x11(81, 1, WHITE, textBuff, Right, updateVRAM, BLACK);
}
/***********************************************************/
static void inputMain(void)
{
	static Input flag;

	switch (globalInput)
	{
	case INPUT_EXT_ADC:
		{
			if (flag != globalInput)
			{
				internalADC_Deinit();
				microphone_Deinit();
				externalADC_Init();
				ST7789_DrawGraphNet_ADC();
				globalNoise = 45;
				globalScale = 1.57;
			}
			sprintf(textBuff, "Extern ADC");
		}
		break;
	case INPUT_INT_ADC:
		{
			if (flag != globalInput)
			{
				externalADC_Deinit();
				microphone_Deinit();
				internalADC_Init();
				ST7789_DrawGraphNet_ADC();
				globalNoise = 45;
				globalScale = 1.57;
			}
			sprintf(textBuff, "Inter ADC ");
		}
		break;
	case INPUT_MICRO:
		{
			if (flag != globalInput)
			{
				internalADC_Deinit();
				externalADC_Deinit();
				microphone_Init();
				ST7789_DrawGraphNet_Mic();
				globalNoise = 77;
				globalScale = 1;
			}
			sprintf(textBuff, "Microphone");
		}
		break;
	}

	flag = globalInput;
	ST7789_DrawText_7x11(81, 1, WHITE, textBuff, Right, updateVRAM, BLACK);
}
/***********************************************************/
static void modeSelect(void)
{
	globalMode += readEncoder();
	constrain((int8_t*) &globalMode, 0, 2);
	modeMain();
}
/***********************************************************/
static void noiseSelect(void)
{
	globalNoise += readEncoder();
	constrain((int8_t*) &globalNoise, 0, 126);
	noiseMain();
}
/***********************************************************/
static void scaleSelect(void)
{
	globalScale += (float) readEncoder() / 10;
	scaleMain();
}
/***********************************************************/
static void inputSelect(void)
{
	globalInput += readEncoder();
	constrain((int8_t*) &globalInput, 0, 2);
	inputMain();
}
/***********************************************************/
static void Generic_Write(const char *Text)
{
	if (Text) ST7789_DrawText_7x11(25, 1, WHITE, (char*) Text, Right, updateVRAM, BLACK);
}
/***********************************************************/
MENU_ITEM(Menu_1, Menu_4, Menu_4, NULL_MENU, Menu_1_1, modeMain, NULL, "Mode: ");
//MENU_ITEM(Menu_2, Menu_3, Menu_1, NULL_MENU, Menu_2_1, noiseMain, NULL, "2.Noise: ");
//MENU_ITEM(Menu_3, Menu_4, Menu_2, NULL_MENU, Menu_3_1, scaleMain, NULL, "3.Scale: ");
MENU_ITEM(Menu_4, Menu_1, Menu_1, NULL_MENU, Menu_4_1, inputMain, NULL, "Input: ");

MENU_ITEM(Menu_1_1, Menu_1_1, Menu_1_1, NULL_MENU, Menu_1, modeSelect, NULL, "Mode->");
MENU_ITEM(Menu_2_1, Menu_2_1, Menu_2_1, NULL_MENU, Menu_2, noiseSelect, NULL, "Noise->");
MENU_ITEM(Menu_3_1, Menu_3_1, Menu_3_1, NULL_MENU, Menu_3, scaleSelect, NULL, "Scale->");
MENU_ITEM(Menu_4_1, Menu_4_1, Menu_4_1, NULL_MENU, Menu_4, inputSelect, NULL, "Input->");

/***********************************************************/
void menuInit()
{
	Menu_SetGenericWriteCallback(Generic_Write);
	Menu_Navigate(&Menu_1);
}

/***********************************************************/
void RTOSstart()
{
	BaseType_t xReturned;

	vSemaphoreCreateBinary(Semaph_data_ready);
	vSemaphoreCreateBinary(Semaph_CPU_load);
	Queue_encoder = xQueueCreate(1, sizeof(int8_t));
	Queue_menu_navigate = xQueueCreate(1, sizeof(ButtonValues));

	if ((Queue_encoder != NULL) && (Semaph_data_ready != NULL) && (Queue_menu_navigate != NULL))
	{
		xReturned = xTaskCreate((void*) led1Task, "Led 1 Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) encoderTask, "Encoder Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
#if (DEBUG_TASK == 1)
		xReturned = xTaskCreate((void*) debugTask, "Debug Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
#endif
		xReturned = xTaskCreate((void*) menuTask, "Menu Task", configMINIMAL_STACK_SIZE * 10, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) buttEncTask, "Button Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) fftTask, "FFT Task", configMINIMAL_STACK_SIZE * 10, NULL, 2, NULL);
		if (xReturned != pdPASS) Error_Handler();

		vTaskStartScheduler();
	}
}

/***********************************************************/
void menuTask(void const *argument)
{
#if (DEBUG_TASK == 1)
	vTaskDelay(500);
#endif
	ButtonValues Value = BUTTON_NONE;

	inputMain();
	menuInit();

	for (;;)
	{
		portBASE_TYPE xStatus;

		//подглядываем за элементом в очереди, т.к. он пригодится в функциях меню
		xStatus = xQueuePeek(Queue_menu_navigate, &Value, 100);

		if (xStatus == pdPASS)
		{
			switch (Value)
			{
			case BUTTON_PREVIOUS:
				Menu_Navigate(MENU_PREVIOUS);
				break;
			case BUTTON_NEXT:
				Menu_Navigate(MENU_NEXT);
				break;
			case BUTTON_PARENT:
				Menu_Navigate(MENU_PARENT);
				break;
			case BUTTON_CHILD:
				Menu_Navigate(MENU_CHILD);
				break;
			case BUTTON_ENTER:
				Menu_EnterCurrentItem();
				break;
			default:
				break;
			}
		}

		xQueueReset(Queue_menu_navigate);
	}
	vTaskDelete(NULL);
}

/***********************************************************/
void buttEncTask(void const *argument)
{
#if (DEBUG_TASK == 1)
	vTaskDelay(500);
#endif
	ButtonValues Value = BUTTON_NONE;

	for (;;)
	{
		if (HAL_GPIO_ReadPin(Butt_enc_GPIO_Port, Butt_enc_Pin) == 0)
		{
			vTaskDelay(10);
			if (HAL_GPIO_ReadPin(Butt_enc_GPIO_Port, Butt_enc_Pin) == 0) Value = BUTTON_CHILD;
			while (HAL_GPIO_ReadPin(Butt_enc_GPIO_Port, Butt_enc_Pin) == 0)
			{
				uint8_t count;
				vTaskDelay(50);
				++count;
				if (count == 20)
				{
					count = 0;
					Value = BUTTON_PARENT;
					break;
				}
			}
			xQueueSendToBack(Queue_menu_navigate, &Value, 10);
			while (HAL_GPIO_ReadPin(Butt_enc_GPIO_Port, Butt_enc_Pin) == 0);
		}
		vTaskDelay(10);
	}
	vTaskDelete(NULL);
}

/***********************************************************/
void encoderTask(void const *argument)
{
#if (DEBUG_TASK == 1)
	vTaskDelay(500);
#endif
	static uint8_t prevCounter = 0;
	ButtonValues Value = BUTTON_NONE;
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

	for (;;)
	{
		uint8_t currCounter = __HAL_TIM_GET_COUNTER(&htim4) / 2;

		if (currCounter != prevCounter)
		{
			if ((currCounter < 50) && (prevCounter - currCounter) > 50) Value = BUTTON_NEXT; //1
			else if ((prevCounter < 50) && (currCounter - prevCounter) > 50) Value = BUTTON_PREVIOUS; //-1
			else Value = (currCounter > prevCounter) ? BUTTON_NEXT : BUTTON_PREVIOUS;

			prevCounter = currCounter;
			xQueueSendToBack(Queue_menu_navigate, &Value, 10);
		}
		vTaskDelay(50);
	}
	vTaskDelete(NULL);
}

/***********************************************************/
void led1Task(void const *argument)
{
#if (DEBUG_TASK == 1)
	vTaskDelay(500);
#endif
	for (;;)
	{
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		vTaskDelay(500);
	}
	vTaskDelete(NULL);
}

/***********************************************************/
#if (DEBUG_TASK == 1)
void debugTask(void const *argument)
{
	vTaskDelay(500);
	HAL_TIM_Base_Start_IT(&htim11);	//to estimate processor load and time consumption of tasks
	for (;;)
	{
		vTaskList(pcWriteBuffer);
		freemem = xPortGetFreeHeapSize();
		vTaskGetRunTimeStats(loadWriteBuffer);

		uint8_t loadCPU;
		xSemaphoreTake(Semaph_CPU_load, portMAX_DELAY);
		loadCPU = CPU_IDLE;
		snprintf(textBuff, sizeof(textBuff), "Load %d%%", loadCPU);
		ST7789_DrawFillRect(lcd_X_max - 70, 10, 11, lcd_Y_max - 10, BLACK, updateVRAM);
		ST7789_DrawText_7x11(10, 60, WHITE, textBuff, Right, updateVRAM);
		vTaskDelay(1000);
	}
	vTaskDelete(NULL);
}
#endif

// Generate a test sample
void generateSampleFloat(float32_t *fx, int16_t ifreq, int32_t iamplitude)
{
// Generate input sine-wave:
	double amplitude = (double) iamplitude; // Maximum = 16383
	double samplesPerSecond = 16000;
	double frequency = (double) ifreq;
	double radiansPerSecond = (2 * M_PI) * frequency;
	double time = 0;

	int16_t i;
	int16_t pointer = 0;

// Generate the required number of samples
	for (i = 0; i < FHT_LEN; i++)
	{
		fx[i] = (amplitude * sin(radiansPerSecond * time));
		time += 1 / samplesPerSecond;
		pointer++;
	}
}

/************************************************************
 *		FFT преобразование и вывод на дисплей
 ***********************************************************/
void fftTask(void const *argument)
{
#if (DEBUG_TASK == 1)
	vTaskDelay(500);
#endif

	arm_rfft_fast_init_f32(&fft_struct, FHT_LEN);

	for (;;)
	{
		static uint8_t State = 0;
		/* Блокирующий семафор, ожидает готового семпла*/
		xSemaphoreTake(Semaph_data_ready, portMAX_DELAY);

		/* 50% sliding window algorithm */
		switch (State)
		{
		case 0:
			{
				State = 1;
				arm_copy_f32(adc_buf, &fft_in_buf_1[FHT_LEN / 2], FHT_LEN / 2);
				arm_copy_f32(&fft_in_buf_1[FHT_LEN / 2], fft_in_buf_2, FHT_LEN / 2);
				arm_copy_f32(fft_in_buf_1, fft_in_buf, FHT_LEN);
				break;
			}
		case 1:
			{
				State = 2;
				arm_copy_f32(adc_buf, fft_in_buf_1, FHT_LEN / 2);
				arm_copy_f32(fft_in_buf_1, &fft_in_buf_2[FHT_LEN / 2], FHT_LEN / 2);
				arm_copy_f32(fft_in_buf_2, fft_in_buf, FHT_LEN);
				break;
			}
		case 2:
			{
				State = 3;
				arm_copy_f32(adc_buf, &fft_in_buf_1[FHT_LEN / 2], FHT_LEN / 2);
				arm_copy_f32(&fft_in_buf_1[FHT_LEN / 2], fft_in_buf_2, FHT_LEN / 2);
				arm_copy_f32(fft_in_buf_1, fft_in_buf, FHT_LEN);
				break;
			}
		case 3:
			{
				State = 0;
				arm_copy_f32(adc_buf, fft_in_buf_1, FHT_LEN / 2);
				arm_copy_f32(fft_in_buf_1, &fft_in_buf_2[FHT_LEN / 2], FHT_LEN / 2);
				arm_copy_f32(fft_in_buf_2, fft_in_buf, FHT_LEN);
				break;
			}
		}

//		static uint16_t i;
//		generateSampleFloat(fft_in_buf, 8, 8388607);
//		i += 10;
//		if (i > 2500) i = 0;

		/* bipolar transform */
		if (globalInput != INPUT_MICRO)
		{
			for (uint16_t i = 0; i < FHT_LEN; ++i)
			{
				fft_in_buf[i] -= 2048.0;
			}
		}
		else
		{
			for (uint16_t i = 0; i < FHT_LEN; ++i)
			{
				if (fft_in_buf[i] > 8388607) fft_in_buf[i] -= 16777216;
			}
		}

		/* Visualisation */
		if (globalMode == MODE_SCOPE)
		{
			if (globalInput != INPUT_MICRO)
			{
				for (uint16_t i = 0; i < ST7789_HEIGHT; ++i)
				{
					ST7789_DrawLine_for_Analyzer(i, (fft_in_buf[i] + 2048) / 32);
				}
			}
			else
			{
				for (uint16_t i = 0; i < ST7789_HEIGHT; ++i)
				{
					ST7789_DrawLine_for_Analyzer(i, (fft_in_buf[i] + 8388607 - 8000000) / 6072); //131072);
				}
			}
		}
		else
		{
//			fhtDitInt((int16_t*) fft_in_buf);
//			complexToDecibelWithGain((int16_t*) fft_in_buf);
//			complexToReal((int16_t*)fft_in_buf, 7);
			applyHammingWindowFloat(fft_in_buf);
			arm_rfft_fast_f32(&fft_struct, (float32_t*) &fft_in_buf, (float32_t*) &fft_out_buf, 0);
			arm_cmplx_mag_f32(fft_out_buf, fft_out_buf, FHT_LEN);

			static int32_t freqs[FHT_LEN / 2];

			for (uint16_t i = 0; i < FHT_LEN / 2; ++i)
			{
				freqs[i] = (int32_t) ((20 * (log10f(fft_out_buf[i]))) - globalNoise) * globalScale;
				if (freqs[i] < 0) freqs[i] = 0;
			}

			if (globalMode == MODE_ANALYZER)
			{
				const uint16_t numBin[35] =
				{ 102, 107, 112, 118, 124, 130, 136, 142, 149, 157, 164, 172, 180, 189, 198, 208, 218, 228, 239, 251,
						263, 276, 289, 303, 318, 333, 349, 366, 384, 402, 422, 442, 464, 486, 510 };

				freqs[0] = 110;
				freqs[102] = 110;
//				freqs[487] = 100;
				freqs[510] = 110;

				for (uint16_t i = 0; i < ST7789_HEIGHT - 26; ++i)
//				for (uint16_t i = 0; i < FHT_LEN / 2; ++i)
				{
//					ST7789_DrawLine_for_Analyzer(i + 26, fft_in_buf[i]);
					if (i > 98) ST7789_DrawLine_for_Analyzer(i + 26, freqs[numBin[i - 99]]);
					else ST7789_DrawLine_for_Analyzer(i + 26, freqs[i]);
				}
			}
			if (globalMode == MODE_AUDIO_SPECTR)
			{
				uint32_t pIndex;
				const uint16_t numBin[25] =
				{ 1, 2, 3, 4, 5, 6, 7, 9, 12, 15, 19, 24, 31, 39, 50, 63, 80, 101, 127, 160, 202, 255, 322, 406, 510 };

				arm_max_f32((float32_t*) &freqs[18], 3, (float32_t*) &freqs[19], &pIndex);	//11
				arm_max_f32((float32_t*) &freqs[21], 5, (float32_t*) &freqs[24], &pIndex);	//12
				arm_max_f32((float32_t*) &freqs[27], 7, (float32_t*) &freqs[31], &pIndex);	//13
				arm_max_f32((float32_t*) &freqs[35], 7, (float32_t*) &freqs[39], &pIndex);	//14
				arm_max_f32((float32_t*) &freqs[46], 9, (float32_t*) &freqs[50], &pIndex);	//15
				arm_max_f32((float32_t*) &freqs[59], 9, (float32_t*) &freqs[63], &pIndex);	//16
				arm_max_f32((float32_t*) &freqs[74], 13, (float32_t*) &freqs[80], &pIndex);	//17
				arm_max_f32((float32_t*) &freqs[93], 17, (float32_t*) &freqs[101], &pIndex);	//18
				arm_max_f32((float32_t*) &freqs[117], 21, (float32_t*) &freqs[127], &pIndex);	//19
				arm_max_f32((float32_t*) &freqs[147], 27, (float32_t*) &freqs[160], &pIndex);	//20
				arm_max_f32((float32_t*) &freqs[187], 31, (float32_t*) &freqs[202], &pIndex);	//21
				arm_max_f32((float32_t*) &freqs[233], 45, (float32_t*) &freqs[255], &pIndex);	//22
				arm_max_f32((float32_t*) &freqs[294], 57, (float32_t*) &freqs[322], &pIndex);	//23
				arm_max_f32((float32_t*) &freqs[370], 73, (float32_t*) &freqs[406], &pIndex);	//24
				arm_max_f32((float32_t*) &freqs[465], 46, (float32_t*) &freqs[510], &pIndex);	//25

				for (uint16_t i = 0; i < 25; ++i)
				{
					ST7789_DrawColumn_for_Audio(i, (int16_t) (freqs[numBin[i]]));
				}
			}
		}
		ST7789_PrintScreen();
	}
	vTaskDelete(NULL);
}

/**********************************************************/
void switchTask()
{
	static portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	/* Отдать семафор задаче-обработчику */
	xSemaphoreGiveFromISR(Semaph_data_ready, &xHigherPriorityTaskWoken);
	if (xHigherPriorityTaskWoken != pdFALSE)
	{
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}
/************************************************************
 *    Конфигурирование каналов DMA, запуск SPI и TIM
 ***********************************************************/
void externalADC_Init()
{
	static uint8_t first_time_flag = 1;

	if (first_time_flag)
	{
		first_time_flag = 0;

		/* Указатель на функцию обратного вызова по окончанию работы DMA */
		hspi2.hdmarx->XferCpltCallback = externalADC_cmpl_callback;
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
	else
	{
		__HAL_SPI_ENABLE(&hspi2);
		__HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	}
}
/**********************************************************/
void externalADC_Deinit()
{
	__HAL_SPI_DISABLE(&hspi2);
	__HAL_TIM_DISABLE_DMA(&htim2, TIM_DMA_UPDATE);
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
}
/***********************************************************
 * 			externalADC_cmpl_callback
 ***********************************************************/
void externalADC_cmpl_callback(DMA_HandleTypeDef *hdma)
{
	if (hdma->Instance == DMA1_Stream3)
	{
		static uint16_t i;

		/* Подготовка данных*/
		adc_buf[i] = (rx_buff & 0x1FFE) >> 1;

		i++;
		if (i == FHT_LEN / 2)
		{
			i = 0;
			switchTask();
		}
	}
}

/************************************************************
 *		      3 function for internal ADC
 ***********************************************************/
void internalADC_Init()
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_buf, FHT_LEN / 2);	//Запуск ДМА от АЦП в память
	HAL_TIM_Base_Start(&htim2);										//Запуск таймера для дискретизации
}
/**********************************************************/
void internalADC_Deinit()
{
	HAL_ADC_Stop_DMA(&hadc1);	//Запуск ДМА от АЦП в память
	HAL_TIM_Base_Stop_IT(&htim2);										//Запуск таймера для дискретизации
}
/**********************************************************/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) raw_adc_data, FHT_LEN / 2);

	for (uint16_t i = 0; i < FHT_LEN / 2; ++i)
	{
		adc_buf[i] = (float32_t) raw_adc_data[i];
	}
	switchTask();
}

/************************************************************
 *		   3 function for i2s mems microphone
 ***********************************************************/
void microphone_Init()
{
	HAL_I2S_Receive_DMA(&hi2s3, raw_adc_data, FHT_LEN);
}
/**********************************************************/
void microphone_Deinit()
{
	HAL_I2S_DMAStop(&hi2s3);
}
/**********************************************************/
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	for (uint16_t j = 0, i = 0; i < FHT_LEN / 2; i++, j = 4 * i)
	{
		adc_buf[i] = (float32_t) ((uint32_t) (raw_adc_data[j] << 8) | (raw_adc_data[j + 1] >> 8));
	}
	switchTask();
}
