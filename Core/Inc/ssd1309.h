/*
 * ssd1309.h
 *
 *  Created on: Jan 20, 2025
 *      Author: JakubLeb
 */

#ifndef INC_SSD1309_H_
#define INC_SSD1309_H_

#include <stdint.h>
#include <stdbool.h>

// OLED display dimensions
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_PAGES (OLED_HEIGHT / 8) // Number of pages of display

// Color definitions
#define BLACK 0
#define WHITE 1

// OLED control functions
void OLED_Reset(void);
void OLED_WriteCommand(uint8_t command);
void OLED_WriteData(uint8_t data);
void OLED_Init(void);

// Drawing functions
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_Fill(uint8_t color);
void OLED_Clear(void);
void OLED_TestPattern(void);

// Image rendering functions
void OLED_PrintImage(const uint8_t *imageData);
void OLED_PrintSmallImageFromCoords(const uint8_t *imageData, uint8_t startX, uint8_t startY,
                                    uint8_t imageWidth, uint8_t imageHeight);
void OLED_PrintImageFromCoords(const uint8_t *imageData, uint8_t startX, uint8_t startY,
                                uint8_t imageWidth, uint8_t imageHeight);

// Shape rendering functions
void OLED_FillRectangle(uint8_t startX, uint8_t startY, uint8_t width, uint8_t height, bool fill);

#endif /* INC_SSD1309_H_ */
