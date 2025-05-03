#include "TELEMETRY.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Initialize Telemetry object
 * @param tm: Pointer to TELEMETRY_S object
 * @param huart: Pointer to UART handle
 * @return int8_t: 0 if successful
 */
int8_t TM_Init(TELEMETRY_S *tm, UART_HandleTypeDef *huart)
{
    tm->UARTHandle = huart;

    // Initialize variables and buffers
    tm->RxIndex = 0;
    memset(tm->RxBuffer, 0, TM_BUFFER_SIZE);

    // Initialize transmission buffer
    tm->TxBuffer.head = 0;
    tm->TxBuffer.tail = 0;
    tm->TxBusy = 0;
    tm->TxCurrentLength = 0;

    // Start receiving data via UART
    HAL_UART_Receive_IT(tm->UARTHandle, &tm->RxByte, 1);

    return 0;
}

/**
 * @brief Handle data received from UART
 * @param huart: UART handle that received data
 * @param tm: Pointer to TELEMETRY_S object
 * @return int8_t: 0 if successful
 */
int8_t TM_Update(UART_HandleTypeDef *huart, TELEMETRY_S *tm)
{
    if (huart->Instance == tm->UARTHandle->Instance)
    {
        // Store received byte in buffer
        tm->RxBuffer[tm->RxIndex++] = tm->RxByte;

        // Prevent buffer overflow
        if (tm->RxIndex >= TM_BUFFER_SIZE)
        {
            tm->RxIndex = 0;
        }

        // Continue receiving next byte
        HAL_UART_Receive_IT(tm->UARTHandle, &tm->RxByte, 1);
    }
    return 0;
}

/**
 * @brief Add data to transmission buffer
 * @param tm: Pointer to TELEMETRY_S object
 * @param data: Data to send
 * @param length: Data length
 * @return int8_t: 0 if successful, 1 if buffer is full
 */
int8_t TM_EnqueueData(TELEMETRY_S *tm, uint8_t *data, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        uint16_t nextHead = (tm->TxBuffer.head + 1) % TM_BUFFER_SIZE;
        if (nextHead == tm->TxBuffer.tail)
            return 1; // Buffer full

        tm->TxBuffer.buffer[tm->TxBuffer.head] = data[i];
        tm->TxBuffer.head = nextHead;
    }
    return 0;
}

/**
 * @brief Set node name
 * @param tm: Pointer to TELEMETRY_S object
 * @param nodeName: Node name
 * @return int8_t: 0 if successful, 1 if error
 */
int8_t TM_SetNodeName(TELEMETRY_S *tm, const char *nodeName)
{
    uint16_t len = strlen(nodeName);
    uint8_t buffer[TM_BUFFER_SIZE];
    uint16_t pos = 0;

    // Format: 0xFF 0x51 [nodeName]
    buffer[pos++] = TM_HEADER_BYTE;
    buffer[pos++] = TM_SET_NODE_NAME_COMMAND;

    // Copy node name to buffer
    memcpy(&buffer[pos], nodeName, len);
    pos += len;

    return TM_EnqueueData(tm, buffer, pos);
}

/**
 * @brief Set mapping between ID and value name
 * @param tm: Pointer to TELEMETRY_S object
 * @param id: Value ID
 * @param name: Value name
 * @return int8_t: 0 if successful, 1 if error
 */
int8_t TM_SetIdAssign(TELEMETRY_S *tm, uint8_t id, const char *name)
{
    uint16_t len = strlen(name);
    uint8_t buffer[TM_BUFFER_SIZE];
    uint16_t pos = 0;

    // Format: 0xFF 0x52 [id] [name]
    buffer[pos++] = TM_HEADER_BYTE;
    buffer[pos++] = TM_SET_ID_ASSIGN_COMMAND;
    buffer[pos++] = id;

    // Copy value name to buffer
    memcpy(&buffer[pos], name, len);
    pos += len;

    return TM_EnqueueData(tm, buffer, pos);
}

/**
 * @brief Set value for an ID
 * @param tm: Pointer to TELEMETRY_S object
 * @param id: Value ID
 * @param value: Double value
 * @return int8_t: 0 if successful, 1 if error
 */
int8_t TM_SetDataField(TELEMETRY_S *tm, uint8_t id, double value)
{
    uint8_t buffer[16]; // Sufficient for header, id and 8 bytes of double
    uint16_t pos = 0;

    // Format: 0xFF 0x53 [id] [value bytes]
    buffer[pos++] = TM_HEADER_BYTE;
    buffer[pos++] = TM_SET_DATA_FIELD_COMMAND;
    buffer[pos++] = id;

    // sending double value as 8 bytes
    uint8_t *valuePtr = (uint8_t *)&value;

    for (int i = 0; i < sizeof(double); i++)
    {
        buffer[pos++] = valuePtr[i];
    }

    return TM_EnqueueData(tm, buffer, pos);
}

/**
 * @brief Request ESP32 to publish data to MQTT
 * @param tm: Pointer to TELEMETRY_S object
 * @return int8_t: 0 if successful, 1 if error
 */
int8_t TM_PublishData(TELEMETRY_S *tm)
{
    uint8_t buffer[2];

    // Format: 0xFF 0x54
    buffer[0] = TM_HEADER_BYTE;
    buffer[1] = TM_PUBLISH_DATA_COMMAND;

    return TM_EnqueueData(tm, buffer, 2);
}

/**
 * @brief Process data transmission from buffer using DMA
 * @param tm: Pointer to TELEMETRY_S object
 * @return void
 */
void TM_Process(TELEMETRY_S *tm)
{
    if (tm->TxBusy == 0)
    {
        uint16_t pending = 0;
        if (tm->TxBuffer.head >= tm->TxBuffer.tail)
            pending = tm->TxBuffer.head - tm->TxBuffer.tail;
        else
            pending = TM_BUFFER_SIZE - tm->TxBuffer.tail;

        if (pending > 0)
        {
            tm->TxBusy = 1;
            tm->TxCurrentLength = pending;
            HAL_UART_Transmit_DMA(tm->UARTHandle,
                                  (uint8_t *)&tm->TxBuffer.buffer[tm->TxBuffer.tail],
                                  pending);
        }
    }
}

/**
 * @brief Callback when UART transmission completes
 * @param tm: Pointer to TELEMETRY_S object
 * @return void
 */
void TM_UART_TxCpltCallback(TELEMETRY_S *tm)
{
    tm->TxBuffer.tail = (tm->TxBuffer.tail + tm->TxCurrentLength) % TM_BUFFER_SIZE;
    tm->TxBusy = 0;
}
