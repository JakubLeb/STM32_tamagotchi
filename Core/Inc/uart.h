/*
 * uart.h
 *
 *  Created on: Jan 17, 2025
 *      Author: JakubLeb
 */

#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include "main.h"

#define USART_TXBUF_LEN 1512
#define USART_RXBUF_LEN 300

extern volatile uint16_t USART_RxBuf[];
extern volatile uint16_t USART_RX_Empty;
extern volatile uint16_t USART_RX_Busy;

extern volatile uint16_t USART_TxBuf[];
extern volatile uint16_t USART_TX_Busy;
extern volatile uint16_t USART_TX_Empty;
extern char frame[];
extern uint16_t frame_id;

uint8_t USART_kbhit(void);
uint8_t USART_getline(char *buf);
int16_t USART_getchar(void);
void USART_fsend(char* format,...);
void UART_Init(void);
void USART_send_response(const char *sender, const char *receiver, const char *response_data);

#endif /* __UART_H */ /* INC_UART_H_ */
