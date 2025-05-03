// Telemetry and Debugging Library for STM32H7xx
#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "stdint.h"
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"

#include "string.h"
#include "stdlib.h"

#define TM_HEADER_BYTE 0xFF

#define TM_SET_NODE_NAME_COMMAND 0x51
#define TM_SET_ID_ASSIGN_COMMAND 0x52
#define TM_SET_DATA_FIELD_COMMAND 0x53
#define TM_PUBLISH_DATA_COMMAND 0x54

#define TM_HEADER buffer[0]
#define TM_COMMAND buffer[1]
#define TM_ID buffer[2]

#define TM_BUFFER_SIZE 1024 // 1K buffer size for telemetry data

typedef struct
{
    char buffer[TM_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} TM_CommandCircularBuffer_t;

typedef struct
{
    UART_HandleTypeDef *UARTHandle; // Handle UART

    uint8_t RxByte;
    uint8_t RxBuffer[TM_BUFFER_SIZE];
    uint16_t RxIndex;

    TM_CommandCircularBuffer_t TxBuffer;
    volatile uint8_t TxBusy;
    volatile uint8_t TxCurrentLength;
} TELEMETRY_S;

int8_t TM_Init(TELEMETRY_S *tm, UART_HandleTypeDef *huart);
int8_t TM_Update(UART_HandleTypeDef *huart, TELEMETRY_S *tm);
int8_t TM_EnqueueData(TELEMETRY_S *tm, uint8_t *data, uint16_t length);
int8_t TM_SetNodeName(TELEMETRY_S *tm, const char *nodeName);
int8_t TM_SetIdAssign(TELEMETRY_S *tm, uint8_t id, const char *name);
int8_t TM_SetDataField(TELEMETRY_S *tm, uint8_t id, double value);
int8_t TM_PublishData(TELEMETRY_S *tm);
void TM_Process(TELEMETRY_S *tm);
void TM_UART_TxCpltCallback(TELEMETRY_S *tm);

#endif // TELEMETRY_H
