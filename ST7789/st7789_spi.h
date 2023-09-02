/*
 * st7789_spi.c
 *
 *  Created on: Aug 18, 2023
 *      Author: Evgeniy
 */
#define CS_HIGH		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
#define CS_LOW		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define DC_HIGH		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET)
#define DC_LOW		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET)

SPI_HandleTypeDef *hspi_lcd;

/**********************************************************************/
void ST7789_SendCmd(uint8_t Cmd)
{
	while (__HAL_SPI_GET_FLAG(hspi_lcd, SPI_SR_BSY));
	DC_LOW;
	CS_LOW;
	HAL_SPI_Transmit_IT(hspi_lcd, &Cmd, 1);
}

/**********************************************************************/
void ST7789_SendData(uint8_t Data)
{
	while (__HAL_SPI_GET_FLAG(hspi_lcd, SPI_SR_BSY));
	DC_HIGH;
	CS_LOW;
	HAL_SPI_Transmit_IT(hspi_lcd, &Data, 1);
}

/**********************************************************************/
void ST7789_SendData_Block(uint8_t *buff, uint32_t size)
{
	while (__HAL_SPI_GET_FLAG(hspi_lcd, SPI_SR_BSY));
	hspi_lcd->Instance->CR1 |= (SPI_DATASIZE_16BIT & SPI_CR1_DFF);
	DC_HIGH;
	CS_LOW;
	HAL_SPI_Transmit_DMA(hspi_lcd, buff, size);
}

/**********************************************************************/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == hspi_lcd)
	{
		CS_HIGH;
		hspi_lcd->Instance->CR1 &= ~(SPI_DATASIZE_16BIT & SPI_CR1_DFF);
	}
}
