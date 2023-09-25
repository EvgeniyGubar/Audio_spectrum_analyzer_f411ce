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

extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;

arm_rfft_fast_instance_f32 fft_struct;

int16_t rx_buff, tx_buff;		//tx_buff фиктивная переменная для запуска работы SPI на прием
float32_t adc_buf[FHT_LEN / 2];
float32_t fft_in_buf_1[FHT_LEN], fft_in_buf_2[FHT_LEN];
float32_t fft_in_buf[FHT_LEN];
float32_t fft_out_buf[FHT_LEN];

xSemaphoreHandle Semaph_data_ready;
xSemaphoreHandle Semaph_CPU_load;
xQueueHandle Queue_encoder;
xQueueHandle Queue_menu_navigate;

void SPI_DMAReceiveCplt_from_ADC(DMA_HandleTypeDef *hdma);
void led1Task(void const *argument);
void encoderTask(void const *argument);
void debugTask(void const *argument);
void processingTask(void const *argument);
void menuTask(void const *argument);
void buttEncTask(void const *argument);

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

Mode globalMode = MODE_ANALYZER;

uint8_t globalOffset = 0;
float globalScale = 1;
uint8_t globalInput = 0;

volatile static uint8_t CPU_IDLE = 0;

char pcWriteBuffer[1024];
char loadWriteBuffer[1024];
uint32_t freemem;
char buff[30];

extern uint16_t lcd_X_max, lcd_Y_max;

/***********************************************************/
uint8_t GetCPU_IDLE()
{
	return CPU_IDLE;
}
/***********************************************************/
void vApplicationIdleHook()
{
	static portTickType LastTick = 0;
	static uint32_t count;			//наш трудяга счетчик
	static uint32_t max_count;//максимальное значение счетчика, вычисляется при калибровке и соответствует 100% CPU idle

	count++;						//приращение счетчика

	if (xTaskGetTickCount() - LastTick > 400)
	{ //если прошло 1000 тиков (1 сек для моей платфрмы)
		LastTick = xTaskGetTickCount();
		if (count > max_count) max_count = count;         //это калибровка
		CPU_IDLE = 100 * (max_count - count) / max_count;               //вычисляем текущую загрузку
		count = 0;               //обнуляем счетчик
		xSemaphoreGive(Semaph_CPU_load);
	}
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
	switch (globalMode) {
	case MODE_ANALYZER:
		sprintf(buff, "Analyzer");
		break;
	case MODE_SCOPE:
		sprintf(buff, "Scope");
		break;
	case MODE_AUDIO_SPECTR:
		sprintf(buff, "Audio");
		break;
	}
	ST7789_DrawText_7x11(82, 10, WHITE, buff, Right, updateScreen);
}
/***********************************************************/
static void noiseMain(void)
{
	snprintf(buff, sizeof(buff), "%d", globalOffset);
	ST7789_DrawText_7x11(82, 10, WHITE, buff, Right, updateScreen);
}
/***********************************************************/
static void scaleMain(void)
{
	snprintf(buff, sizeof(buff), "%0.1f", globalScale);
	ST7789_DrawText_7x11(82, 10, WHITE, buff, Right, updateScreen);
}
/***********************************************************/
static void inputMain(void)
{
	snprintf(buff, sizeof(buff), "%d", globalInput);
	ST7789_DrawText_7x11(82, 10, WHITE, buff, Right, updateScreen);
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
	globalOffset += readEncoder();
	constrain((int8_t*) &globalOffset, 0, 100);
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

/** Generic function to write the text of a menu.
 *  \param[in] Text   Text of the selected menu to write
 */
static void Generic_Write(const char *Text)
{
	if (Text)
	{
		ST7789_DrawFillRect(lcd_X_max - 20, 10, 11, lcd_Y_max - 10, BLACK, updateVRAM);
		ST7789_DrawText_7x11(10, 10, WHITE, (char*) Text, Right, updateScreen);
	}
}

MENU_ITEM(Menu_1, Menu_2, Menu_4, NULL_MENU, Menu_1_1, modeMain, NULL, "1.Mode:");
MENU_ITEM(Menu_2, Menu_3, Menu_1, NULL_MENU, Menu_2_1, noiseMain, NULL, "2.Noise:");
MENU_ITEM(Menu_3, Menu_4, Menu_2, NULL_MENU, Menu_3_1, scaleMain, NULL, "3.Scale:");
MENU_ITEM(Menu_4, Menu_1, Menu_3, NULL_MENU, Menu_4_1, inputMain, NULL, "4.Input:");

MENU_ITEM(Menu_1_1, Menu_1_1, Menu_1_1, NULL_MENU, Menu_1, modeSelect, NULL, "1.Mode->");
MENU_ITEM(Menu_2_1, Menu_2_1, Menu_2_1, NULL_MENU, Menu_2, noiseSelect, NULL, "2.Noise->");
MENU_ITEM(Menu_3_1, Menu_3_1, Menu_3_1, NULL_MENU, Menu_3, scaleSelect, NULL, "3.Scale->");
MENU_ITEM(Menu_4_1, Menu_4_1, Menu_4_1, NULL_MENU, Menu_4, inputSelect, NULL, "4.Input->");

/***********************************************************/
void menuInit()
{
	Menu_SetGenericWriteCallback(Generic_Write);
	Menu_Navigate(&Menu_1);
}

/***********************************************************/
void RTOScreate()
{
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

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
		xReturned = xTaskCreate((void*) debugTask, "Debug Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) menuTask, "Menu Task", configMINIMAL_STACK_SIZE * 10, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) buttEncTask, "Button enc Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) processingTask, "FFT Task", configMINIMAL_STACK_SIZE * 10, NULL, 2, &xHandle);
		if (xReturned != pdPASS) Error_Handler();

		vTaskStartScheduler();
	}
}

/***********************************************************/
void menuTask(void const *argument)
{
	vTaskDelay(500);
	ButtonValues Value = BUTTON_NONE;
	menuInit();

	for (;;)
	{
		portBASE_TYPE xStatus;

		//подглядываем за элементом в очереди, т.к. он пригодится в функциях меню
		xStatus = xQueuePeek(Queue_menu_navigate, &Value, 100);

		if (xStatus == pdPASS)
		{
			switch (Value) {
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
		//если никто не прочел элемент, то удаляем его
		xQueueReset(Queue_menu_navigate);
	}
	vTaskDelete(NULL);
}

/***********************************************************/
void buttEncTask(void const *argument)
{
	vTaskDelay(500);
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
	vTaskDelay(500);
	static uint8_t prevCounter = 0;
	ButtonValues Value = BUTTON_NONE;

	for (;;)
	{
		uint8_t currCounter = __HAL_TIM_GET_COUNTER(&htim4) / 2;

		if (currCounter != prevCounter)
		{
			if ((currCounter < 50) && (prevCounter - currCounter) > 50)
			{
				Value = BUTTON_NEXT; //1
			}
			else if ((prevCounter < 50) && (currCounter - prevCounter) > 50)
			{
				Value = BUTTON_PREVIOUS; //-1
			}
			else
			{
				Value = (currCounter > prevCounter) ? BUTTON_NEXT : BUTTON_PREVIOUS;
			}

//			char buff[16];
//			snprintf(buff, sizeof(buff), "curr %3d", currCounter);
//			ST7789_print_7x11(10, 10, WHITE, BLACK, 0, buff);

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
	vTaskDelay(500);
	for (;;)
	{
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		vTaskDelay(500);
	}
	vTaskDelete(NULL);
}

/***********************************************************/
void debugTask(void const *argument)
{
	vTaskDelay(500);
	for (;;)
	{
//		vTaskList(pcWriteBuffer);
//		freemem = xPortGetFreeHeapSize();
//		vTaskGetRunTimeStats(loadWriteBuffer);

		uint8_t loadCPU;
		char buff[16];
		xSemaphoreTake(Semaph_CPU_load, portMAX_DELAY);
		loadCPU = CPU_IDLE;
		snprintf(buff, sizeof(buff), "Load %d%%", loadCPU);
//		ST7789_DrawFillRect(10, 60, 128, 11, BLACK, updateVRAM);
		ST7789_DrawFillRect(lcd_X_max - 70, 10, 11, lcd_Y_max - 10, BLACK, updateVRAM);
		ST7789_DrawText_7x11(10, 60, WHITE, buff, Right, updateScreen);
		vTaskDelay(1000);
	}
	vTaskDelete(NULL);
}

/************************************************************
 *		FFT преобразование и вывод на дисплей
 ***********************************************************/
void processingTask(void const *argument)
{
	vTaskDelay(500);
	for (;;)
	{
		/* Блокирующий семафор, ожидает готового семпла*/
		xSemaphoreTake(Semaph_data_ready, portMAX_DELAY);

		if (globalMode == MODE_SCOPE)
		{
			for (uint16_t i = 0; i < ST7789_HEIGHT; ++i)
			{
				ST7789_DrawLine_for_Analyzer(i, (fft_in_buf[i] + 2047) / 32);
			}
		}
		if (globalMode != MODE_SCOPE)
		{
			applyHammingWindowFloat(fft_in_buf);
			arm_rfft_fast_f32(&fft_struct, (float32_t*) &fft_in_buf, (float32_t*) &fft_out_buf, 0);
			arm_cmplx_mag_f32(fft_out_buf, fft_out_buf, FHT_LEN);

			static int freqs[FHT_LEN / 2];
			for (uint16_t i = 0; i < FHT_LEN / 2; ++i)
			{
				//temp = (int) ((20 * (log10f(fft_out_buf[i]))) - offset);
				freqs[i] = (int) ((20 * (log10f(fft_out_buf[i]))) - globalOffset) * globalScale;
				//if (temp > freqs[i]) freqs[i] = temp;
				//else freqs[i]--;
				if (freqs[i] < 0) freqs[i] = 0;
			}

			if (globalMode == MODE_ANALYZER)
			{
				for (uint16_t i = 0; i < ST7789_HEIGHT; ++i)
				{
					ST7789_DrawLine_for_Analyzer(i, freqs[i]);
				}
			}
			if (globalMode == MODE_AUDIO_SPECTR)
			{
				uint32_t pIndex;
				const uint16_t numBin[25] = { 1, 2, 3, 4, 5, 6, 7, 9, 12, 15, 19, 24, 31, 39, 50, 63, 80, 101, 127, 160,
						202, 255, 322, 406, 510 };

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

/***********************************************************/
void fftInit()
{
	arm_rfft_fast_init_f32(&fft_struct, FHT_LEN);
}

/************************************************************
 *    Конфигурирование каналов DMA, запуск SPI и TIM
 ***********************************************************/
void hardwareInit()
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
