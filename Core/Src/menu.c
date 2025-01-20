#include "st7735.h"
#include "stdlib.h"
#include "usart.h"


int menu_handle_input(int* status)
{
	uint8_t pressed_button;
	if (HAL_UART_Receive(&huart2, &pressed_button, 1, 5) == HAL_OK)
	{
        if (pressed_button == 's')
        {
    		ST7735_FillScreen(ST7735_CASET);
            *status = (*status + 1) % 2;  // Correctly update the value pointed to by 'status'
        }
        else if (pressed_button == 'w')
        {
    		ST7735_FillScreen(ST7735_CASET);
            *status = (*status - 1 + 2) % 2;  // Ensure wrapping around with proper precedence
        }
        else if (pressed_button == 'z')
        {
    		ST7735_FillScreen(ST7735_CASET);

			// 0 is game 1 is highscores
        	// but it should be 1 and 2
            return *status + 1;  // Return the current status
        }

        return 0;  // Successfully handled input
	}
	return 0;
}

void menu_draw(int option)
{
	if (option == 0)
	{
		ST7735_DrawString(5, 5, "> Game", ST7735_GREEN);
		ST7735_DrawString(5, 6, "Highscores", ST7735_WHITE);
	}
	else if (option == 1)
	{
		ST7735_DrawString(5, 5, "Game", ST7735_WHITE);
		ST7735_DrawString(5, 6, "> Highscores", ST7735_GREEN);

	}
}

int menu()
{
	int option = 0;
	ST7735_FillScreen(ST7735_CASET);


	while(1)
	{
		int status = menu_handle_input(&option);
		if (status != 0)
		{
			return status;
		}

		menu_draw(option);
	}

}
