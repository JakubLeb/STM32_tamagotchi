/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>

#include "images.h"
#include "bh1750.h"
#include "ssd1309.h"
#include "uart.h"
#include "game_logic_and_animation.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STM "STM"
#define START_ID_SENDER 0
#define START_ID_RECIPIENT 3
#define START_ID_LENGTH_COMMEND 6
#define LENGTH_OF_FRAME_WITHOUT_LENGHT_OF_DATA 13
#define START_ID_DATA 9
#define CMD_MAX_LENGTH 85
#define MAX_FRAME_SIZE 268
#define POLYNOMIAL 0x8005 // Polynomial for CRC-16/ARC
#define INITIAL_CRC 0x0000 // Initial value
#define FINAL_XOR 0x0000 // Final XOR Value
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
typedef struct {
    char sender_frame[4];
    char recipient_frame[4];
    char length_of_command_frame[3];
    char data_frame[257];
    char crc[5];
} Frame_Data;

typedef enum {
    Waiting, // Waiting for frame
    Start, // Start of frame
} FrameDetection;

// CRC table for CRC-16/ARC
static const uint16_t crc_table[256] = {0x0000, 0xC0C1, 0xC181, 0x0140,   0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741,   0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41,   0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40,   0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941,   0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40,   0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540,   0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341,   0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141,   0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740,   0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40,   0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41,   0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940,   0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41,   0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541,   0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340,   0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141,   0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740,   0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40,   0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41,   0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940,   0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41,   0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541,   0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340,   0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140,   0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741,   0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41,   0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40,   0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941,   0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40,   0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540,   0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341,   0x4100, 0x81C1, 0x8081, 0x4040
};
Frame_Data frame_data = {
    .sender_frame = "",
    .recipient_frame = "",
    .length_of_command_frame = "",
    .data_frame = "",
    .crc = ""
};

FrameDetection detection = Waiting;
uint16_t frame_length = 0;
uint16_t frame_id = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
uint8_t USART_kbhit();
int16_t USART_getchar();
uint8_t USART_getline(char *buf);
void USART_fsend(char* format,...);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
uint8_t USART_receive_frame(uint8_t *data, uint16_t *length);
void USART_send_frame(uint8_t *data, uint16_t length);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Function for handling UART transmission completion
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart2) {
        if (USART_TX_Empty != USART_TX_Busy) {
            uint8_t tmp = USART_TxBuf[USART_TX_Busy];
            USART_TX_Busy++;
            if (USART_TX_Busy >= USART_TXBUF_LEN) USART_TX_Busy = 0;
            HAL_UART_Transmit_IT(&huart2, &tmp, 1);
        }
    }
}
// Function for handling UART data reception
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart2) {
        USART_RX_Empty++;
        if (USART_RX_Empty >= USART_RXBUF_LEN) USART_RX_Empty = 0;
        HAL_UART_Receive_IT(&huart2, &USART_RxBuf[USART_RX_Empty], 1);
    }
}

// Function for calculating CRC-16
uint16_t crc16(const uint8_t *data, size_t length) {
    uint16_t crc = INITIAL_CRC;

    for (size_t i = 0; i < length; i++) {
        uint8_t byte = data[i] ^ (crc & 0xFF);
        crc = (crc >> 8) ^ crc_table[byte];
    }

    return crc ^ FINAL_XOR;
}
// Function for verifying CRC
bool verify_crc(const uint16_t *data, size_t length, const char *crc_ascii) {
    // Check if input pointers are not NULL
    if (data == NULL || crc_ascii == NULL) {
        return false;
    }

    // Calculate CRC for given data using crc16 function
    uint16_t calculated_crc = crc16(data, length);

    // Convert ASCII hex to uint16_t
    uint16_t provided_crc = 0;
    for(int i = 0; i < 4; i++) {
        uint8_t val;
        char c = crc_ascii[i];

        // Convert single hex character
        if(c >= '0' && c <= '9') {
            val = c - '0';
        }
        else if(c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        }
        else {
            return false;  // Invalid character
        }

        // Shift and add new value
        provided_crc = (provided_crc << 4) | val;
    }

    return (calculated_crc == provided_crc);
}
// Function for reading frame
void FrameRd() {
    int16_t sign;

    if (USART_RX_Empty == USART_RX_Busy) { // Check if data is available
        return;
    }

    sign = USART_RxBuf[USART_RX_Busy];
    USART_RX_Busy++;

    if (USART_RX_Busy >= USART_RXBUF_LEN) {
        USART_RX_Busy = 0;
    }

    // Check for starting character { and verify if in data field
    // the character is not escaped
    if (sign == '{' && !(frame_id > 9 && frame[frame_id - 1] == '\\')) {
        detection = Start;
        frame_id = 0; // Reset frame length
        return;
    }

    if (detection == Start) {
        // Check for ending character } and verify if in data field
        // the character is not escaped
        if (sign == '}' && !(frame_id > 9 && frame[frame_id - 1] == '\\')) {
            frame_length = frame_id;
            frame_id = 0;
            decoding();
            detection = Waiting; // Return to waiting for new frame
            return;
        }

        frame[frame_id] = sign; // Add current character to frame
        frame_id++;

        // Check maximum frame length
        if (frame_id > (MAX_FRAME_SIZE)) {
            frame_id = 0;
            detection = Waiting; // Reset on invalid frame
            return;
        }
    }
}

// Function for decoding frame
void decoding() {
    // 13 is the minimum frame size without {} characters
    if (frame_length < 13) {
        // Frame is ignored, doesn't meet requirements
        return;
    }

    // Copy sender
    for (int i = 0; i < 3; i++) {
        frame_data.sender_frame[i] = frame[START_ID_SENDER + i];
    }

    // Copy and compare recipient
    bool recipient_match = true;
    for (int i = 0; i < 3; i++) {
        frame_data.recipient_frame[i] = frame[START_ID_RECIPIENT + i];
        if (frame_data.recipient_frame[i] != STM[i]) {
                    recipient_match = false;
                    break;
        }
    }
    if (!recipient_match) {
        return;
    }

    // Copy length and check if it's a number in base 10
    for (int symbol_id = 0; symbol_id < 3; symbol_id++) {
        frame_data.length_of_command_frame[symbol_id] = frame[START_ID_LENGTH_COMMEND + symbol_id];
        if (!(frame_data.length_of_command_frame[symbol_id] >= '0' &&
              frame_data.length_of_command_frame[symbol_id] <= '9')) {
            USART_send_response(STM, frame_data.sender_frame, "WRLENDATA");
            return;
        }
    }

    int cmdLength = 0;
    // Convert to int
    for (int i = 0; i < 3; i++) {
        if (!(frame_data.length_of_command_frame[i] - '0' >= 0 && frame_data.length_of_command_frame[i] - '0' <= 9 )){
            USART_send_response(STM, frame_data.sender_frame, "WRCMD");
            return;
        }
        cmdLength = cmdLength * 10 + (frame_data.length_of_command_frame[i] - '0');
    }

    if (cmdLength >= 256 && cmdLength <=0){
        USART_send_response(STM, frame_data.sender_frame, "WRCMD");
        return;
    }

    // Copy data
    for (int i = 0; i < cmdLength; i++) {
        frame_data.data_frame[i] = frame[9 + i];
    }

    // Copy CRC
    for (int i = 0; i < 4; i++) {
        frame_data.crc[i] = frame[9 + cmdLength + i];
    }

    // Check if CRC is a hexadecimal number
    for (int symbol_id = 0; symbol_id < 4; symbol_id++) {
        if (!(frame_data.crc[symbol_id] >= '0' && frame_data.crc[symbol_id] <= '9') &&
            !(frame_data.crc[symbol_id] >= 'A' && frame_data.crc[symbol_id] <= 'F')) {
            USART_send_response(STM, frame_data.sender_frame, "WRCRC");
            return;
        }
    }

    if (verify_crc((uint16_t *)frame, frame_length-4, frame_data.crc)) {
        parse_and_execute_command(frame_data.data_frame);
    } else {
        USART_send_response(STM, frame_data.sender_frame, "WRCRC");
    }
}

// Function for parsing and executing commands
void parse_and_execute_command(const char* data, int data_length) {
    char header[CMD_MAX_LENGTH] = {0};
    char command[CMD_MAX_LENGTH] = {0};
    char value[CMD_MAX_LENGTH] = {0};

    int header_index = 0, command_index = 0, value_index = 0;
    int i = 0;

    // Parse header (until '_' character)
    while (i < data_length && data[i] != '_') {
        header[header_index++] = data[i++];
    }
    i++; // Skip '_' character

    // Parse command (until '[' character)
    while (i < data_length && data[i] != '[') {
        command[command_index++] = data[i++];
    }
    i++; // Skip '[' character

    // Parse value (until ']' character)
    while (i < data_length && data[i] != ']') {
        value[value_index++] = data[i++];
    }

    // Check if all parts were parsed correctly
    if (header_index == 0 || command_index == 0 || value_index == 0) {
        USART_send_response(STM, frame_data.sender_frame, "WRCMD");
        return;
    }

    // Execute command
    Commends(header, header_index, command, command_index, value, value_index);
}
// Function to check if value is a valid number
bool isNumber(const char* value, int length) {
    for (int i = 0; i < length; i++) {
        if (!isdigit((unsigned char)value[i])) {
            return false;  // If character is not a digit, return false
        }
    }
    return true;  // All characters are digits
}
// Function to convert string to integer if valid, otherwise return -1
int handleNumber(const char* value, int length) {
    if (isNumber(value, length)) {
        int number = 0;
        for (int i = 0; i < length; i++) {
            number = number * 10 + (value[i] - '0');  // Manual conversion to number
        }
        return number;
    }
    return -1;  // If value is not a number, return -1
}

// Function to check if value is within specified range (inclusive)
bool isInRange(int value, int min, int max) {
    return value >= min && value <= max;  // Returns true if value is between min and max
}
// Function for comparing character arrays
int custom_strncmp(const char* str1, const char* str2, int length1, int length2) {
    if (length1 != length2){
        return -1;
    }
    // Compare characters
    for (int i = 0; i < length1; i++) {
        if (str1[i] != str2[i]) {
            return (unsigned char)str1[i] - (unsigned char)str2[i];  // Return difference between characters
        }
    }

    return 0; // Arrays are identical
}
// Function to check if given value is valid depending on head and instruction
bool value_check(const char* head, int head_length, const char* instruction, int instruction_length, const char* value, int value_length) {
    int value_to_int = handleNumber(value, value_length);  // Convert value to number

    // Compare head with "FOOD", "ENERGY", "FUN"
    if (custom_strncmp(head, "FOOD", head_length, 4) == 0 ||
        custom_strncmp(head, "ENERGY", head_length, 6) == 0 ||
        custom_strncmp(head, "FUN", head_length, 3) == 0) {

        // Check for ADD or SUBTRACT instruction
        if ((custom_strncmp(instruction, "ADD", instruction_length, 3) == 0 ||
             custom_strncmp(instruction, "SUBTRACT", instruction_length, 8) == 0) &&
            isInRange(value_to_int, 1, 5)) {
            return true;
        }
        // Check for SET instruction
        if (custom_strncmp(instruction, "SET", instruction_length, 3) == 0 &&
            isInRange(value_to_int, 0, 5)) {
            return true;
        }
        // Check for CLEAR instruction
        if (custom_strncmp(instruction, "CLEAR", instruction_length, 5) == 0 &&
            value[0] == 'X') {
            return true;
        }
    }
    // Compare head with "MENU"
    else if (custom_strncmp(head, "MENU", head_length, 4) == 0) {
        // Check for CHANGE instruction
        if (custom_strncmp(instruction, "CHANGE", instruction_length, 6) == 0 &&
            isInRange(value_to_int, 1, 3)) {
            return true;
        }
    }
    // Compare head with "WAKEUP"
    else if (custom_strncmp(head, "WAKEUP", head_length, 6) == 0) {
        // Check value range
        if (isInRange(value_to_int, 1, 65535)) {
            return true;
        }
    }

    return false;  // If no condition is met, return false
}

// Main function for processing commands and executing appropriate actions
void Commends(const char* head, int head_length, const char* instruction, int instruction_length, const char* value, int value_length) {
    uint16_t* attribute = NULL;

    // Handle FOOD, ENERGY, FUN
    if (custom_strncmp(head, "FOOD", head_length, 4) == 0 ||
        custom_strncmp(head, "ENERGY", head_length, 6) == 0 ||
        custom_strncmp(head, "FUN", head_length, 3) == 0) {

        if (custom_strncmp(head, "FOOD", head_length, 4) == 0) {
            attribute = &g_atribute.food;
        } else if (custom_strncmp(head, "ENERGY", head_length, 6) == 0) {
            attribute = &g_atribute.energy;
        } else if (custom_strncmp(head, "FUN", head_length, 3) == 0) {
            attribute = &g_atribute.fun;
        }

        if (!attribute) {
            USART_send_response(STM, frame_data.sender_frame, "WRCMD");
            return;
        }

        // Check if menu is in game state (action blocked)
        if (g_atribute.menu == 3) {
            USART_send_response(STM, frame_data.sender_frame, "WRWAITGAME");
            return;
        }

        // Handle ADD instruction
        if (custom_strncmp(instruction, "ADD", instruction_length, 3) == 0) {
            if (g_atribute.minigame) {
                USART_send_response(STM, frame_data.sender_frame, "WRWAITGAME");
                return;
            }
            if (value_check(head, head_length, instruction, instruction_length, value, value_length)) {
                int new_value = *attribute + handleNumber(value, value_length);
                *attribute = (new_value > 5) ? 5 : new_value;  // Limit value to 5
                USART_send_response(STM, frame_data.sender_frame, "ACCEPTADDAT");
            } else {
                USART_send_response(STM, frame_data.sender_frame, "WRVALUE");
            }
        }
        // Handle SUBTRACT instruction
        else if (custom_strncmp(instruction, "SUBTRACT", instruction_length, 8) == 0) {
            if (g_atribute.minigame) {
                USART_send_response(STM, frame_data.sender_frame, "WRWAITGAME");
                return;
            }
            if (value_check(head, head_length, instruction, instruction_length, value, value_length)) {
                int new_value = *attribute - handleNumber(value, value_length);
                *attribute = (new_value < 0) ? 0 : new_value;  // Limit value to 0
                USART_send_response(STM, frame_data.sender_frame, "ACCEPTSUBAT");
            } else {
                USART_send_response(STM, frame_data.sender_frame, "WRVALUE");
            }
        }
        // Handle SET instruction
        else if (custom_strncmp(instruction, "SET", instruction_length, 3) == 0) {
            if (g_atribute.minigame) {
                USART_send_response(STM, frame_data.sender_frame, "WRWAITGAME");
                return;
            }
            if (value_check(head, head_length, instruction, instruction_length, value, value_length)) {
                *attribute = handleNumber(value, value_length);
                USART_send_response(STM, frame_data.sender_frame, "ACCEPTSET");
            } else {
                USART_send_response(STM, frame_data.sender_frame, "WRVALUE");
            }
        }
        // Handle CLEAR instruction
        else if (custom_strncmp(instruction, "CLEAR", instruction_length, 5) == 0) {
            if (g_atribute.minigame) {
                USART_send_response(STM, frame_data.sender_frame, "WRWAITGAME");
                return;
            }
            if (value_check(head, head_length, instruction, instruction_length, value, value_length)) {
                *attribute = 0;  // Reset value to 0
                USART_send_response(STM, frame_data.sender_frame, "ACCEPTRESAT");
            } else {
                USART_send_response(STM, frame_data.sender_frame, "WRVALUE");
            }
        }
        // Unknown instruction
        else {
            USART_send_response(STM, frame_data.sender_frame, "WRCMD");
        }
    }
    // Handle MENU
    else if (custom_strncmp(head, "MENU", head_length, 4) == 0) {
        if (g_atribute.minigame) {
            USART_send_response(STM, frame_data.sender_frame, "WRMENU");
            return;
        }

        // Handle CHANGE instruction
        if (custom_strncmp(instruction, "CHANGE", instruction_length, 6) == 0) {
            if (value_check(head, head_length, instruction, instruction_length, value, value_length)) {
                g_atribute.menu = handleNumber(value, value_length);  // Change menu state
                USART_send_response(STM, frame_data.sender_frame, "ACCEPTMENU");
            } else {
                USART_send_response(STM, frame_data.sender_frame, "WRVALUE");
            }
        }
        // Unknown instruction
        else {
            USART_send_response(STM, frame_data.sender_frame, "WRCMD");
        }
    }
    // Handle WAKEUP
    else if (custom_strncmp(head, "WAKEUP", head_length, 6) == 0) {
        if (g_atribute.minigame) {
            USART_send_response(STM, frame_data.sender_frame, "WRWAITGAME");
            return;
        }

        // Handle MIN instruction
        if (custom_strncmp(instruction, "MIN", instruction_length, 3) == 0) {
            if (value_check(head, head_length, instruction, instruction_length, value, value_length)) {
                g_atribute.wakeup_sensitivity = handleNumber(value, value_length);  // Set wakeup sensitivity
                USART_send_response(STM, frame_data.sender_frame, "ACCEPTSET");
            } else {
                USART_send_response(STM, frame_data.sender_frame, "WRVALUE");
            }
        }
        // Unknown instruction
        else {
            USART_send_response(STM, frame_data.sender_frame, "WRCMD");
        }
    }
    // Unknown head
    else {
        USART_send_response(STM, frame_data.sender_frame, "WRCMD");
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  // Initialize UART data reception in interrupt mode
  HAL_UART_Receive_IT(&huart2,&USART_RxBuf[USART_RX_Empty],1);

  // Initialize BH1750 sensor (light sensor)
  BH1750_Init();

  // Start measurement by BH1750 sensor
  BH1750_StartMeasurement();

  // Initialize OLED display
  OLED_Init();

  // Start and update first menu
  Start_menu1();
  Update_menu1();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){
      Button_Functionality(); // Handle main buttons
      Menu_change_and_update(); // Change menu and update it
      FrameRd(); // Read data frames

  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GreenBt_Pin RedBt_Pin YellowBt_Pin BlueBt_Pin */
  GPIO_InitStruct.Pin = GreenBt_Pin|RedBt_Pin|YellowBt_Pin|BlueBt_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : OLED_RES_Pin */
  GPIO_InitStruct.Pin = OLED_RES_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(OLED_RES_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OLED_DC_Pin */
  GPIO_InitStruct.Pin = OLED_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(OLED_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OLED_CS_Pin */
  GPIO_InitStruct.Pin = OLED_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(OLED_CS_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
