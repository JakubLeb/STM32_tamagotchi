/*
 * uart.c
 *
 *  Created on: Jan 20, 2025
 *      Author: JakubLeb
 */

#include "uart.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "main.h"

// Buffers for receiving and transmitting data via USART
volatile uint16_t USART_RxBuf[USART_RXBUF_LEN]; // Receive buffer
volatile uint16_t USART_RX_Empty = 0;           // Receive index (empty)
volatile uint16_t USART_RX_Busy = 0;            // Receive index (busy)

volatile uint16_t USART_TxBuf[USART_TXBUF_LEN]; // Transmit buffer
volatile uint16_t USART_TX_Busy = 0;            // Transmit index (busy)
volatile uint16_t USART_TX_Empty = 0;           // Transmit index (empty)
char frame[USART_RXBUF_LEN];                    // Data frame

// Function checking if data is available in the USART receive buffer
uint8_t USART_kbhit(){
	if(USART_RX_Empty == USART_RX_Busy){
		return 0;
	}else{
		return 1;
	}
}

// Function for retrieving a line from the receive buffer
uint8_t USART_getline(char *buf) {
    static uint8_t bf[USART_RXBUF_LEN];
    static uint8_t idx = 0;
    int i;
    uint8_t ret;

    while (USART_kbhit()) {
        bf[idx] = USART_getchar();

        if ((bf[idx] == '\n') || (bf[idx] == '\r')) {
            bf[idx] = '\0';
            for (i = 0; i <= idx; i++) {
                buf[i] = bf[i];
            }
            ret = idx;
            idx = 0;
            return ret;
        } else {
            idx++;
            if (idx >= USART_RXBUF_LEN) {
                idx = 0;
            }
        }
    }
    return 0;
}

// Function for retrieving a single character from the receive buffer
int16_t USART_getchar(){
    int16_t tmp;

	if(USART_RX_Empty != USART_RX_Busy){
        tmp = USART_RxBuf[USART_RX_Busy];
        USART_RX_Busy++;
        if(USART_RX_Busy >= USART_RXBUF_LEN) USART_RX_Busy = 0;
        return tmp;
	}else return -1;
}

// Function for sending data via USART (with formatting)
void USART_send_frame(const char *frame, int frame_idx) {
    // Index in the transmit buffer
    int idx = USART_TX_Empty;

    // Copying data to the circular buffer
    for(int i = 0; i < frame_idx; i++) {
        USART_TxBuf[idx] = frame[i];
        idx++;
        // Ensuring buffer circularity
        if(idx >= USART_TXBUF_LEN) {
            idx = 0;
        }
    }

    // Disable interrupts before modifying shared variables
    __disable_irq();

    // If buffer is empty and UART ready for transmission
    if((USART_TX_Empty == USART_TX_Busy) && (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE) == SET)) {
        // Update end of data pointer
        USART_TX_Empty = idx;
        // Get first byte to send
        uint8_t tmp = USART_TxBuf[USART_TX_Busy];
        // Move read pointer
        USART_TX_Busy++;
        // Ensure buffer circularity
        if(USART_TX_Busy >= USART_TXBUF_LEN) {
            USART_TX_Busy = 0;
        }
        // Start transmission of first byte
        HAL_UART_Transmit_IT(&huart2, &tmp, 1);
    } else {
        // If transmission in progress, just update end pointer
        USART_TX_Empty = idx;
    }

    // Enable interrupts
    __enable_irq();
}

// Function for sending response
void USART_send_response(const char *sender, const char *receiver, const char *response_data) {
    char frame[USART_TXBUF_LEN];
    uint16_t data_length = 0;
    uint16_t crc;
    int frame_idx = 0;

    frame[frame_idx++] = '{';

    // Copy sender
    for (int i = 0; i < 3; i++) {
        frame[frame_idx++] = sender[i];
    }

    // Copy receiver
    for (int i = 0; i < 3; i++) {
        frame[frame_idx++] = receiver[i];
    }

    // Calculate data length
    while (response_data[data_length] && data_length < 256) {
        data_length++;
    }
    // Convert length to 3 decimal digits
    frame[frame_idx++] = '0' + (data_length / 100);
    frame[frame_idx++] = '0' + ((data_length / 10) % 10);
    frame[frame_idx++] = '0' + (data_length % 10);

    // Copy response data
    for (int i = 0; i < data_length && frame_idx < (USART_TXBUF_LEN - 6); i++) {
        frame[frame_idx++] = response_data[i];
    }

    // Calculate CRC for data between { and CRC
    crc = crc16((uint8_t*)frame + 1, frame_idx - 1);

    // Add CRC in hexadecimal format
    frame[frame_idx++] = "0123456789ABCDEF"[(crc >> 12) & 0x0F];
    frame[frame_idx++] = "0123456789ABCDEF"[(crc >> 8) & 0x0F];
    frame[frame_idx++] = "0123456789ABCDEF"[(crc >> 4) & 0x0F];
    frame[frame_idx++] = "0123456789ABCDEF"[crc & 0x0F];

    // End frame with '}' character
    frame[frame_idx++] = '}';
    USART_send_frame(frame, frame_idx);
}
