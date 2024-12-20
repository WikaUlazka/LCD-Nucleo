#ifndef LCD_H
#define LCD_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx_hal.h"

// Rozdzielczość wyświetlacza
#define LCD_WIDTH        128
#define LCD_HEIGHT       160

// Definicje kolorów (z odwróconą kolejnością bajtów)
#define BLACK            0x0000
#define RED              0x00f8
#define GREEN            0xe007
#define BLUE             0x1f00
#define YELLOW           0xe0ff
#define MAGENTA          0x1ff8
#define CYAN             0xff07
#define WHITE            0xffff

// Definicje pinów i portów LCD (dostosuj do swojej konfiguracji)
#define LCD_CS_PIN       GPIO_PIN_6    // Chip Select pin (dostosuj do swojej konfiguracji)
#define LCD_CS_PORT      GPIOB       // Chip Select port (dostosuj do swojej konfiguracji)
#define LCD_DC_PIN       GPIO_PIN_8    // Data/Command pin (dostosuj do swojej konfiguracji)
#define LCD_DC_PORT      GPIOB         // Data/Command port (dostosuj do swojej konfiguracji)
#define LCD_RST_PIN      GPIO_PIN_7    // Reset pin (dostosuj do swojej konfiguracji)
#define LCD_RST_PORT     GPIOB         // Reset port (dostosuj do swojej konfiguracji)

// Deklaracje funkcji
void lcd_init(void);                // Inicjalizacja wyświetlacza
void lcd_put_pixel(int x, int y, uint16_t color);  // Ustawienie piksela
void lcd_copy(void);                // Przesłanie zawartości bufora na LCD
void lcd_transfer_done(void);       // Sprawdzenie, czy transfer zakończony
int lcd_is_busy(void);             // Sprawdzenie, czy wyświetlacz jest zajęty

void lcd_put_char(int x, int y, char c, uint16_t color);  // Funkcja wyświetlająca pojedynczy znak

#endif /* LCD_H */
