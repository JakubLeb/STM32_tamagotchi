/*
 * bh1750.c
 *
 *  Created on: Jan 20, 2025
 *      Author: JakubLeb
 */

#include "main.h"
#include "bh1750.h"

// Buffer for data received from the BH1750 sensor
static uint8_t rx_buffer[2];

// Function initializing the BH1750 sensor
void BH1750_Init(void)
{
    uint8_t cmd = BH1750_CMD_POWER_ON;  // Power-on command

    // Sending the power-on command to the sensor via I2C
    HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDRESS, &cmd, 1, HAL_MAX_DELAY);

    HAL_Delay(200);  // Delay for sensor stabilization after powering on
}

// Function starting light intensity measurement in continuous mode
void BH1750_StartMeasurement(void)
{
    uint8_t cmd = BH1750_CMD_CONT_H_MODE;  // Command to start continuous high-resolution measurement

    // Sending the command to the sensor via I2C to start measurement
    HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDRESS, &cmd, 1, HAL_MAX_DELAY);
}

// Function reading light intensity in lux
float BH1750_ReadLux(void)
{
    // Receiving two bytes of data from the BH1750 sensor
    HAL_I2C_Master_Receive(&hi2c1, BH1750_ADDRESS, rx_buffer, 2, HAL_MAX_DELAY);

    // Combining two bytes into one 16-bit result
    uint16_t raw_data = (rx_buffer[0] << 8) | rx_buffer[1]; // Merging bytes into a 16-bit value

    // Converting raw data to lux value (in default mode)
    float lux = (float)raw_data / 1.2;  // Conversion to lux in default mode

    return lux;  // Returning the lux value
}
