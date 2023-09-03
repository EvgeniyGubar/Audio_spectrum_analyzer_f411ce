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

int16_t rx_buff, tx_buff; //tx_buff фиктивная переменная для запуска работы SPI на прием
float32_t adc_buf[FHT_LEN / 2];
float32_t fft_in_buf_1[FHT_LEN], fft_in_buf_2[FHT_LEN];
float32_t fft_in_buf[FHT_LEN];
float32_t fft_out_buf[FHT_LEN];

xSemaphoreHandle Semaph_data_ready;
xQueueHandle Queue_encoder;
xQueueHandle Queue_menu_navigate;

void SPI_DMAReceiveCplt_from_ADC(DMA_HandleTypeDef *hdma);
void led1Task(void const *argument);
void encoderTask(void const *argument);
void debugTask(void const *argument);
void processingTask(void const *argument);
void menuTask(void const *argument);
void buttEncTask(void const *argument);

/*** DUMMY CODE ***/
typedef enum {
	BUTTON_NONE, BUTTON_PREVIOUS, BUTTON_NEXT, BUTTON_PARENT, BUTTON_CHILD, BUTTON_ENTER,
} ButtonValues;

ButtonValues GetButtonPress(void)
{
	return BUTTON_NONE;
}
/*** END DUMMY CODE ***/

/** Example menu item specific enter callback function, run when the associated menu item is entered. */
static void Level1Item1_Enter(void)
{
	ST7789_DrawFillRect(10, 30, 128, 11, BLACK, updateVRAM);
	ST7789_DrawText_7x11(10, 30, WHITE, "Enter", updateScreen);
//	puts("ENTER");
}

/** Example menu item specific select callback function, run when the associated menu item is selected. */
static void Level1Item1_Select(void)
{
	ST7789_DrawFillRect(10, 30, 128, 11, BLACK, updateVRAM);
	ST7789_DrawText_7x11(10, 30, WHITE, "Select", updateScreen);
//	puts("SELECT");
}

/** Generic function to write the text of a menu.
 *
 *  \param[in] Text   Text of the selected menu to write, in \ref MENU_ITEM_STORAGE memory space
 */
static void Generic_Write(const char *Text)
{
	if (Text)
	{
		ST7789_DrawFillRect(10, 10, 128, 11, BLACK, updateVRAM);
		ST7789_DrawText_7x11(10, 10, WHITE, (char*)Text, updateScreen);
	}
//		puts(Text);
}

MENU_ITEM(Menu_1, Menu_2, Menu_3, NULL_MENU, Menu_1_1, Level1Item1_Select, Level1Item1_Enter, "Func 1");
MENU_ITEM(Menu_2, Menu_3, Menu_1, NULL_MENU, NULL_MENU, NULL, NULL, "Func 2");
MENU_ITEM(Menu_3, Menu_1, Menu_2, NULL_MENU, NULL_MENU, NULL, NULL, "Func 3");

MENU_ITEM(Menu_1_1, Menu_1_2, Menu_1_2, Menu_1, NULL_MENU, NULL, NULL, "1.1");
MENU_ITEM(Menu_1_2, Menu_1_1, Menu_1_1, Menu_1, NULL_MENU, NULL, NULL, "1.2");

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
	Queue_encoder = xQueueCreate(1, sizeof(int8_t));
	Queue_menu_navigate = xQueueCreate(5, sizeof(ButtonValues));

	if ((Queue_encoder != NULL) && (Semaph_data_ready != NULL) && (Queue_menu_navigate != NULL))
	{
		xReturned = xTaskCreate((void*) led1Task, "Led 1 Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) encoderTask, "Encoder Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) debugTask, "Debug Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) menuTask, "Menu Task", configMINIMAL_STACK_SIZE*10, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) buttEncTask, "Button enc Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
		if (xReturned != pdPASS) Error_Handler();
		xReturned = xTaskCreate((void*) processingTask, "Processing Task", configMINIMAL_STACK_SIZE * 10, NULL, 1, &xHandle);
		if (xReturned != pdPASS) Error_Handler();

		vTaskStartScheduler();
	}
}

/***********************************************************/
void menuTask(void const *argument)
{
	for (;;)
	{
		/* Example usage of MicroMenu - here you can create your custom menu navigation system; you may wish to perform
		 * other tasks while detecting key presses, enter sleep mode while waiting for user input, etc.
		 */
		ButtonValues Value = BUTTON_NONE;
		portBASE_TYPE xStatus;

		xStatus = xQueueReceive(Queue_menu_navigate, &Value, 0);

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
	}
	vTaskDelete(NULL);
}

/***********************************************************/
void buttEncTask(void const *argument)
{
	for (;;)
	{
		ButtonValues Value = BUTTON_NONE;

		if (HAL_GPIO_ReadPin(Butt_enc_GPIO_Port, Butt_enc_Pin) == 0)
		{
			vTaskDelay(20);
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
		vTaskDelay(50);
	}
	vTaskDelete(NULL);
}

/***********************************************************/
void encoderTask(void const *argument)
{
	static uint8_t prevCounter = 0;
	ButtonValues Value = BUTTON_NONE;

	for (;;)
	{
		uint8_t currCounter = __HAL_TIM_GET_COUNTER(&htim4)/2;

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
//			snprintf(buff, sizeof(buff), "prev %3d", prevCounter);
//			ST7789_print_7x11(10, 25, WHITE, BLACK, 0, buff);
//			snprintf(buff, sizeof(buff), "enc %3d", encoder);
//			ST7789_print_7x11(10, 40, WHITE, BLACK, 0, buff);
			prevCounter = currCounter;
			xQueueSendToBack(Queue_menu_navigate, &Value, 10);
			vTaskDelay(50);
		}
	}
	vTaskDelete(NULL);
}

/***********************************************************/
void led1Task(void const *argument)
{
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
	for (;;)
	{
		vTaskDelay(500);
	}
	vTaskDelete(NULL);
}

/************************************************************
 *		FFT преобразование и вывод на дисплей
 ***********************************************************/
void processingTask(void const *argument)
{
//	portBASE_TYPE xStatus;
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

			applyHammingWindowFloat(fft_in_buf);
			arm_rfft_fast_f32(&fft_struct, (float32_t*) &fft_in_buf, (float32_t*) &fft_out_buf, 0);
			arm_cmplx_mag_f32(fft_out_buf, fft_out_buf, FHT_LEN);


//			int temp;
			static int freqs[FHT_LEN / 2];
//			int8_t offset, encoder;	//variable noisefloor offset
//			/* Берем информацию о состоянии энкодера из очереди */
//			xStatus = xQueueReceive(Queue_encoder, &encoder, 0);
//
//			if (xStatus == pdPASS)
//			{
//				offset += encoder;
//				if (offset < 0) offset = 0;
//			}

			for (uint16_t i = 0; i < FHT_LEN / 2; ++i)
			{
//				temp = (int) ((20 * (log10f(fft_out_buf[i]))) - offset);

				freqs[i] = (int) (20 * (log10f(fft_out_buf[i]))); // - offset;
//				if (temp > freqs[i]) freqs[i] = temp;
//				else freqs[i]--;

				if (freqs[i] < 0) freqs[i] = 0;
			}

			for (uint16_t i = 0; i < ST7789_HEIGHT; ++i)
			{
				ST7789_DrawLine_for_Analyzer(i, freqs[i]);
			}

//			static uint16_t numBin[25] = { 1, 2, 3, 4, 5, 6, 7, 9, 12, 15, 19, 24, 31, 39, 50, 63, 80, 101, 127, 160,
//					202, 255, 322, 406, 510 };
//
//			for (uint16_t i = 0; i < 25; ++i)
//			{
//				ST7789_DrawLine_for_Analyzer(4 + i * 6, (int16_t) (freqs[numBin[i]] * 1.5));
//			}
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
