#include "hagl.h"
#include "lcd.h"
#include "font6x9.h" // Zakładając, że masz zdefiniowaną czcionkę w formie bitmapy

// Funkcja do rysowania wypełnionego prostokąta na ekranie
void hagl_fill_rectangle(int x0, int y0, int x1, int y1, uint16_t color) {
    for (int x = x0; x <= x1; x++) {
        for (int y = y0; y <= y1; y++) {
            lcd_set_pixel(x, y, color);  // Rysowanie piksela
        }
    }
}

// Funkcja do wyświetlania tekstu na ekranie
void hagl_put_text(const char *text, int x, int y, uint16_t color, const uint8_t *font) {
    int i = 0;
    while (text[i] != '\0') {
        uint8_t letter = text[i] - ' '; // Mapowanie znaków od spacji (ASCII 32)
        for (int j = 0; j < 6; j++) {   // Zakładamy czcionkę 6x8
            uint8_t column = font[letter * 6 + j];
            for (int bit = 0; bit < 8; bit++) {
                if (column & (1 << bit)) {
                    lcd_set_pixel(x + j, y + bit, color); // Rysowanie piksela
                }
            }
        }
        x += 6; // Przesunięcie na następny znak
        i++;
    }
}
