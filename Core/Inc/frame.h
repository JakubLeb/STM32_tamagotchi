/*
 * frame.h
 *
 *  Created on: Jan 17, 2025
 *      Author: JakubLeb
 */

#ifndef __FRAME_H
#define __FRAME_H

#include <stdbool.h>

#define PC "PC0"
#define STM "STM"

typedef struct {
    char recipient_frame[3];
    int id_recipient_frame;
    char sender_frame[3];
    int id_sender_frame;
    char length_of_commend_frame[3];
    int id_length_of_commend_frame;
    char data_frame[256];
    int id_data_frame;
    char crc[4];
    int id_crc;
} Frame_Data;

typedef enum {
    Waiting,
    Recipient,
    Sender,
    Length_of_commend,
    Data,
    Crc,
    End
} FrameDetection;

typedef struct {
    bool recipient_check;
    bool sender_check;
    bool Length_of_commend_detect_check;
    bool data_check;
    bool crc_check;
} Frame_Check;

void FrameRd(void);
void parse_and_execute_command(const char* data);
void USART_send_response(const char *sender, const char *receiver, const char *response_data);
uint16_t crc16_arc(uint8_t *data, uint16_t length);

#endif /* __FRAME_H */
