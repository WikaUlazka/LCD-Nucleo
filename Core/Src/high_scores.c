#include "st7735.h"
#include "stdlib.h"
#include "usart.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"
#include "high_scores.h"

void Flash_Write(uint64_t* data, uint32_t length) {
    HAL_FLASH_Unlock();

    // Erase the sector
    FLASH_EraseInitTypeDef eraseInitStruct;
    uint32_t sectorError = 0;

    eraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInitStruct.Banks = FLASH_BANK_2;  // Adjust for your target sector
    eraseInitStruct.Page = 0;
    eraseInitStruct.NbPages = 2;

    if (HAL_FLASHEx_Erase(&eraseInitStruct, &sectorError) != HAL_OK) {
        printf("Erase failed! Sector Error: %lu\n", sectorError);
        HAL_FLASH_Lock();
        return;
    }

    // Write data
    for (uint32_t i = 0; i < length; i++) {
        uint64_t data_to_write = data[i];
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, FLASH_USER_START_ADDR + (i * 8), data_to_write) != HAL_OK) {
            printf("Write failed at index %d\n", i);
            HAL_FLASH_Lock();
            return;
        }
    }

    HAL_FLASH_Lock();
}

void Flash_Read(uint64_t* buffer, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        uint64_t data = *(__IO uint64_t *)(FLASH_USER_START_ADDR + (i * 8));
        buffer[i] = data;
    }
}

HAL_StatusTypeDef Flash_WriteArray(uint32_t address, uint64_t *data, uint32_t length)
{
	Flash_Write(data, length);
    return HAL_OK; // Return success if all bytes were written successfully
}


void Flash_ReadArray(uint32_t address, uint64_t *data, uint32_t length)
{
	Flash_Read(data, length);
}


int scores_handle_input()
{
	uint8_t pressed_button;
	if (HAL_UART_Receive(&huart2, &pressed_button, 1, 5) == HAL_OK)
	{
        if (pressed_button == 'z')
        {
    		ST7735_FillScreen(ST7735_CASET);

			// Go do menu
    		return 1;
        }

        return 0;  // Successfully handled input
	}
	return 0;

}

void scores_draw()
{
	uint64_t scores[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    Flash_ReadArray(FLASH_USER_START_ADDR, scores, 10);

    for (int i = 0; i < 10; i++)
    {
    	uint32_t score = scores[i];
    	char buffer[100];
        sprintf(buffer, "%d. %lu", i, score);

    	ST7735_DrawString(5, 3 + i, buffer, ST7735_WHITE);

    }
}

int high_scores()
{
	ST7735_FillScreen(ST7735_CASET);

	scores_draw();

	while (1)
	{
		int status = scores_handle_input();
		if (status != 0)
		{
			return 0;
		}
	}
	return 0;
}
