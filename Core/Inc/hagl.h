#ifndef HAGL_H
#define HAGL_H

#include <stdint.h>
#include "lcd.h" // Wykorzystuje funkcje z `lcd.h`

void hagl_fill_rectangle(int x0, int y0, int x1, int y1, uint16_t color); // Rysowanie prostokąta
void hagl_put_text(const char *text, int x, int y, uint16_t color, const uint8_t *font); // Wyświetlanie tekstu

#endif // HAGL_H
