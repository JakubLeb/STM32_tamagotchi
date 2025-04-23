/*
 * ssd1309.c
 *
 *  Created on: Jan 20, 2025
 *      Author: JakubLeb
 */
#include "ssd1309.h"
#include <stdbool.h>
#include "main.h"

// Display buffer - stores the current state of pixels
static uint8_t displayBuffer[OLED_PAGES][OLED_WIDTH] = {0};

// Resets the display by manipulating the RES pin
void OLED_Reset(void) {
    HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_SET);
}

// Sends a command to the display via SPI
void OLED_WriteCommand(uint8_t command) {
    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);  // Command mode
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET);  // Select OLED
    HAL_SPI_Transmit(&hspi1, &command, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);    // Deselect OLED
}

// Sends data to the display via SPI
void OLED_WriteData(uint8_t data) {
    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);    // Data mode
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET);  // Select OLED
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);    // Deselect OLED
}

// Display initialization - configuration of all necessary parameters
void OLED_Init(void) {
    OLED_Reset();
    HAL_Delay(10);

    OLED_WriteCommand(0xAE); // Turn off display

    OLED_WriteCommand(0xD5); // Set display clock
    OLED_WriteCommand(0x80); // Recommended values

    OLED_WriteCommand(0xA8); // Multiplex ratio
    OLED_WriteCommand(0x3F); // 1/64 duty cycle

    OLED_WriteCommand(0xD3); // Display offset
    OLED_WriteCommand(0x00); // No offset

    OLED_WriteCommand(0x40); // Start line = 0

    OLED_WriteCommand(0x8D); // Charge pump
    OLED_WriteCommand(0x14); // Enable charge pump

    OLED_WriteCommand(0x20); // Memory mode
    OLED_WriteCommand(0x00); // Horizontal addressing

    OLED_WriteCommand(0xA0); // Segment mapping (0 to 127)
    OLED_WriteCommand(0xC0); // COM scan direction

    OLED_WriteCommand(0xDA); // COM pins
    OLED_WriteCommand(0x12); // COM pins configuration

    OLED_WriteCommand(0x81); // Contrast
    OLED_WriteCommand(0xFF); // Maximum contrast

    OLED_WriteCommand(0xA4); // Display from RAM
    OLED_WriteCommand(0xA6); // Normal display (not inverted)

    OLED_WriteCommand(0xAF); // Turn on display
}

// Draws a single pixel on the display
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;

    uint8_t page = y / 8;     // Calculate page (vertical byte)
    uint8_t bit = y % 8;      // Calculate bit in the page

    // Set or clear specific bit
    if (color == WHITE) {
        displayBuffer[page][x] |= (1 << bit);  // Set bit to 1
    } else {
        displayBuffer[page][x] &= ~(1 << bit); // Set bit to 0
    }

    // Update display
    OLED_WriteCommand(0xB0 | page);
    OLED_WriteCommand(0x00 | (x & 0x0F));
    OLED_WriteCommand(0x10 | ((x >> 4) & 0x0F));
    OLED_WriteData(displayBuffer[page][x]);
}

// Fills the entire display with one color
void OLED_Fill(uint8_t color) {
    uint8_t fillValue = (color == WHITE) ? 0xFF : 0x00;

    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        OLED_WriteCommand(0xB0 | page);
        OLED_WriteCommand(0x00);
        OLED_WriteCommand(0x10);

        for (uint8_t x = 0; x < OLED_WIDTH; x++) {
            displayBuffer[page][x] = fillValue;
            OLED_WriteData(fillValue);
        }
    }
}

// Clears the display (fills with black color)
void OLED_Clear(void) {
    OLED_Fill(BLACK);
}

// Test pattern - draws a frame
void OLED_TestPattern(void) {
    OLED_Clear();

    for(uint8_t x = 0; x < OLED_WIDTH; x++) {
        OLED_DrawPixel(x, 0, WHITE);          // Top line
        OLED_DrawPixel(x, OLED_HEIGHT-1, WHITE); // Bottom line
    }

    for(uint8_t y = 0; y < OLED_HEIGHT; y++) {
        OLED_DrawPixel(0, y, WHITE);          // Left line
        OLED_DrawPixel(OLED_WIDTH-1, y, WHITE); // Right line
    }
}

// Displays a full image from data array
void OLED_PrintImage(const uint8_t *imageData) {
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        OLED_WriteCommand(0xB0 | page);
        OLED_WriteCommand(0x00);
        OLED_WriteCommand(0x10);

        for (uint8_t x = 0; x < OLED_WIDTH; x++) {
            uint8_t byte = imageData[page * OLED_WIDTH + x];
            displayBuffer[page][x] = byte;
            OLED_WriteData(byte);
        }
    }
}

// Displays a small image from given coordinates (optimized version)
void OLED_PrintSmallImageFromCoords(const uint8_t *imageData, uint8_t startX, uint8_t startY,
                              uint8_t imageWidth, uint8_t imageHeight) {
    for (uint8_t y = 0; y < imageHeight; y++) {
        uint8_t displayY = startY + y;
        if (displayY >= OLED_HEIGHT) continue;

        uint8_t page = displayY / 8;
        uint8_t bit = displayY % 8;

        for (uint8_t x = 0; x < imageWidth; x++) {
            uint8_t displayX = startX + x;
            if (displayX >= OLED_WIDTH) continue;

            uint8_t imageByteIndex = (y / 8) * imageWidth + x;
            uint8_t imageBitIndex = y % 8;

            uint8_t pixelValue = (imageData[imageByteIndex] & (1 << imageBitIndex)) ? 1 : 0;

            if (pixelValue) {
                displayBuffer[page][displayX] |= (1 << bit);
            } else {
                displayBuffer[page][displayX] &= ~(1 << bit);
            }

            OLED_WriteCommand(0xB0 | page);
            OLED_WriteCommand(0x00 | (displayX & 0x0F));
            OLED_WriteCommand(0x10 | ((displayX >> 4) & 0x0F));
            OLED_WriteData(displayBuffer[page][displayX]);
        }
    }
}

// Displays an image from given coordinates
void OLED_PrintImageFromCoords(const uint8_t *imageData, uint8_t startX, uint8_t startY,
                              uint8_t imageWidth, uint8_t imageHeight) {
    uint8_t imagePages = (imageHeight + 7) / 8;  // Number of 8-pixel pages in the image

    for (uint8_t page = 0; page < imagePages; page++) {
        for (uint8_t x = 0; x < imageWidth; x++) {
            uint8_t imageByte = imageData[page * imageWidth + x];

            for (uint8_t bit = 0; bit < 8; bit++) {
                uint8_t displayY = startY + (page * 8) + bit;
                uint8_t displayX = startX + x;

                if (displayY >= OLED_HEIGHT || displayX >= OLED_WIDTH) continue;

                uint8_t pixelValue = (imageByte & (1 << bit)) ? 1 : 0;

                uint8_t displayPage = displayY / 8;
                uint8_t displayBit = displayY % 8;

                if (pixelValue) {
                    displayBuffer[displayPage][displayX] |= (1 << displayBit);
                } else {
                    displayBuffer[displayPage][displayX] &= ~(1 << displayBit);
                }

                OLED_WriteCommand(0xB0 | displayPage);
                OLED_WriteCommand(0x00 | (displayX & 0x0F));
                OLED_WriteCommand(0x10 | ((displayX >> 4) & 0x0F));
                OLED_WriteData(displayBuffer[displayPage][displayX]);
            }
        }
    }
}

// Fills a rectangle with given dimensions
void OLED_FillRectangle(uint8_t startX, uint8_t startY, uint8_t width, uint8_t height, bool fill) {
    if (startX >= OLED_WIDTH || startY >= OLED_HEIGHT) return;
    if ((startX + width) > OLED_WIDTH) width = OLED_WIDTH - startX;
    if ((startY + height) > OLED_HEIGHT) height = OLED_HEIGHT - startY;

    for (uint8_t y = startY; y < (startY + height); y++) {
        uint8_t page = y / 8;
        uint8_t bit = y % 8;

        for (uint8_t x = startX; x < (startX + width); x++) {
            if (fill) {
                displayBuffer[page][x] |= (1 << bit);     // White pixel
            } else {
                displayBuffer[page][x] &= ~(1 << bit);    // Black pixel
            }

            OLED_WriteCommand(0xB0 | page);
            OLED_WriteCommand(0x00 | (x & 0x0F));
            OLED_WriteCommand(0x10 | ((x >> 4) & 0x0F));
            OLED_WriteData(displayBuffer[page][x]);
        }
    }
}
