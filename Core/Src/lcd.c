#include "lcd.h"
#include "spi.h"
#include "main.h"
#include <stdbool.h>
#include "stm32l4xx_hal.h"

extern SPI_HandleTypeDef hspi1;  // Używamy hspi1 zamiast hspi2

// Definicje komend i funkcji do komunikacji z ekranem
#define ST7735S_SLPOUT        	0x11
#define ST7735S_DISPOFF        	0x28
#define ST7735S_DISPON        	0x29
#define ST7735S_CASET        	0x2a
#define ST7735S_RASET        	0x2b
#define ST7735S_RAMWR        	0x2c
#define ST7735S_MADCTL        	0x36
#define ST7735S_COLMOD        	0x3a
#define ST7735S_FRMCTR1        	0xb1
#define ST7735S_FRMCTR2        	0xb2
#define ST7735S_FRMCTR3        	0xb3
#define ST7735S_INVCTR        	0xb4
#define ST7735S_PWCTR1        	0xc0
#define ST7735S_PWCTR2        	0xc1
#define ST7735S_PWCTR3        	0xc2
#define ST7735S_PWCTR4        	0xc3
#define ST7735S_PWCTR5        	0xc4
#define ST7735S_VMCTR1        	0xc5
#define ST7735S_GAMCTRP1    	0xe0
#define ST7735S_GAMCTRN1    	0xe1

// Funkcje pomocnicze do komunikacji
static void lcd_cmd(uint8_t cmd) {
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);  // Komenda
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);  // Chip select
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);  // Zmieniono z hspi2 na hspi1
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);  // Chip deselect
}

static void lcd_data(uint8_t data) {
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);  // Dane
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);  // Chip select
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);  // Zmieniono z hspi2 na hspi1
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);  // Chip deselect
}

static void lcd_data16(uint16_t value) {
    lcd_data(value >> 8);  // Wyślij wyższy bajt
    lcd_data(value);       // Wyślij niższy bajt
}

#define CMD(x) ((x) | 0x100)

static void lcd_send(uint16_t value) {
    if (value & 0x100) {
        lcd_cmd(value);  // Komenda
    } else {
        lcd_data(value);  // Dane
    }
}

// Inicjalizacja ekranu
static const uint16_t init_table[] = {
    CMD(ST7735S_FRMCTR1), 0x01, 0x2c, 0x2d,
    CMD(ST7735S_FRMCTR2), 0x01, 0x2c, 0x2d,
    CMD(ST7735S_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
    CMD(ST7735S_INVCTR), 0x07,
    CMD(ST7735S_PWCTR1), 0xa2, 0x02, 0x84,
    CMD(ST7735S_PWCTR2), 0xc5,
    CMD(ST7735S_PWCTR3), 0x0a, 0x00,
    CMD(ST7735S_PWCTR4), 0x8a, 0x2a,
    CMD(ST7735S_PWCTR5), 0x8a, 0xee,
    CMD(ST7735S_VMCTR1), 0x0e,
    CMD(ST7735S_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
    CMD(ST7735S_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
    CMD(ST7735S_COLMOD), 0x05,
    CMD(ST7735S_MADCTL), 0xa0,
};

// Offsety LCD
#define LCD_OFFSET_X 1
#define LCD_OFFSET_Y 2

// Ustawienie okna LCD
static void lcd_set_window(int x, int y, int width, int height) {
    lcd_cmd(ST7735S_CASET);
    lcd_data16(LCD_OFFSET_X + x);
    lcd_data16(LCD_OFFSET_X + x + width - 1);

    lcd_cmd(ST7735S_RASET);
    lcd_data16(LCD_OFFSET_Y + y);
    lcd_data16(LCD_OFFSET_Y + y + height - 1);
}

// Bufor do przechowywania danych wyświetlacza
static uint16_t frame_buffer[LCD_WIDTH * LCD_HEIGHT];

// Funkcje do pracy z pikselami
void lcd_put_pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < LCD_WIDTH && y >= 0 && y < LCD_HEIGHT) { // Sprawdzenie granic
        frame_buffer[x + y * LCD_WIDTH] = color;
    }
}

void lcd_copy(void) {
    lcd_set_window(0, 0, LCD_WIDTH, LCD_HEIGHT);
    lcd_cmd(ST7735S_RAMWR);
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)frame_buffer, sizeof(frame_buffer));  // Zmieniono z hspi2 na hspi1
}

void lcd_init(void) {
    int i;

    // Reset ekranu
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(100);

    // Wysyłanie komend inicjalizacyjnych
    for (i = 0; i < sizeof(init_table) / sizeof(uint16_t); i++) {
        lcd_send(init_table[i]);
    }
    HAL_Delay(200);

    // Włączenie wyświetlacza
    lcd_cmd(ST7735S_SLPOUT);
    HAL_Delay(120);
    lcd_cmd(ST7735S_DISPON);
}
void lcd_set_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) {
        return; // Ignoruj, jeśli piksel jest poza ekranem
    }

    frame_buffer[y * LCD_WIDTH + x] = color; // Ustaw piksel w buforze
}

// Funkcja zakończenia transmisji SPI
void lcd_transfer_done(void) {
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

// Funkcja sprawdzająca, czy ekran jest zajęty
int lcd_is_busy(void) {
    if (HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY) {  // Zmieniono z hspi2 na hspi1
        return 1;
    } else {
        return 0;
    }
}

// Tablica fontu 5x7
static const uint8_t font_5x7[26][5] = {
    {0x7E, 0x09, 0x09, 0x09, 0x7E},  // A
    {0x7F, 0x49, 0x49, 0x49, 0x36},  // B
    {0x3E, 0x41, 0x41, 0x41, 0x22},  // C
    {0x7F, 0x41, 0x41, 0x41, 0x3E},  // D
    {0x7F, 0x49, 0x49, 0x49, 0x41},  // E
    {0x7F, 0x09, 0x09, 0x09, 0x01},  // F
    {0x3E, 0x41, 0x49, 0x49, 0x2E},  // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F},  // H
    {0x41, 0x41, 0x7F, 0x41, 0x41},  // I
    {0x20, 0x40, 0x40, 0x40, 0x3F},  // J
    {0x7F, 0x08, 0x14, 0x22, 0x41},  // K
    {0x7F, 0x40, 0x40, 0x40, 0x40},  // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F},  // M
    {0x7F, 0x02, 0x04, 0x08, 0x7F},  // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E},  // O
    {0x7F, 0x09, 0x09, 0x09, 0x06},  // P
    {0x3E, 0x41, 0x41, 0x51, 0x3E},  // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46},  // R
    {0x26, 0x49, 0x49, 0x49, 0x32},  // S
    {0x01, 0x01, 0x7F, 0x01, 0x01},  // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F},  // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F},  // V
    {0x3F, 0x40, 0x30, 0x40, 0x3F},  // W
    {0x63, 0x14, 0x08, 0x14, 0x63},  // X
    {0x07, 0x08, 0x70, 0x08, 0x07},  // Y
    {0x61, 0x51, 0x49, 0x45, 0x43},  // Z
};

void lcd_put_char(int x, int y, char c, uint16_t color) {
    if (c < 'A' || c > 'Z') return;  // Obsługuje tylko litery A-Z

    for (int i = 0; i < 5; i++) {
        uint8_t column = font_5x7[c - 'A'][i];
        for (int bit = 0; bit < 7; bit++) {
            if (column & (1 << bit)) {
                lcd_put_pixel(x + i, y + bit, color);
            }
        }
    }
}
