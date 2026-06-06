/*	UART Example

	This example code is in the Public Domain (or CC0 licensed, at your option.)

	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "SensorPacket.h"

extern MessageBufferHandle_t xMessageBufferRx;
extern MessageBufferHandle_t xMessageBufferTx;
extern size_t xItemSize;


// CRC8 function form STM32:
static uint8_t crc8(const uint8_t *buf, size_t len)
{
    uint8_t crc = 0;

    while (len--)
    {
        crc ^= *buf++;

        for (int i = 0; i < 8; i++)
        {
            crc = (crc & 0x80)
                ? ((crc << 1) ^ 0x07)
                : (crc << 1);
        }
    }

    return crc;
}

// Payload length helper funciton:
static size_t getPayloadLength(uint8_t type, uint8_t count)
{
    switch(type)
    {
        case ADC_COMBINED:
            return count * 6;

        case EMG:
        case EEG:
        case EKG:
            return count * 2;

        case MAX1030x:
        case MAX_Sp02:
        case MAX_HR:
            return count * 10;

        default:
            return 0;
    }
}

void uart_tx(void* pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start using GPIO%d", CONFIG_UART_TX_GPIO);

	char buffer[xItemSize];
	while(1) {
		size_t received = xMessageBufferReceive(xMessageBufferTx, buffer, sizeof(buffer), portMAX_DELAY);
		ESP_LOGI(pcTaskGetName(NULL), "xMessageBufferReceive received=%d", received);
		if (received > 0) {
			ESP_LOGD(pcTaskGetName(NULL), "xMessageBufferReceive buffer=[%.*s]",received, buffer);
			ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(NULL), buffer, received, ESP_LOG_INFO);
			int txBytes = uart_write_bytes(UART_NUM_1, buffer, received);
			if (txBytes != received) {
				ESP_LOGE(pcTaskGetName(NULL), "uart_write_bytes Fail. txBytes=%d received=%d", txBytes, received);
			}
		}
	} // end while

	// Never reach here
	vTaskDelete(NULL);
}

void uart_rx(void* pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start using GPIO%d", CONFIG_UART_RX_GPIO);

	SensorPacket packet;

	while(1) {
		uint8_t byte;
		//---------------------------------------------------------
		// Wait for sync byte 1 from stm
		//---------------------------------------------------------
		if(uart_read_bytes(UART_NUM_1, &byte,1,portMAX_DELAY) != 1) {
			continue;
		}

		if(byte != UART_SYNC1) {continue;}

		//---------------------------------------------------------
		// Wait for sync byte 2 from stm
		//---------------------------------------------------------
		if(uart_read_bytes(UART_NUM_1, &byte, 1, portMAX_DELAY) != 1) {
			continue;
		}

		if(byte != UART_SYNC2) {continue;}

		//---------------------------------------------------------
		// Read Header
		//---------------------------------------------------------
		uint8_t hdr[6];
		if(uart_read_bytes(UART_NUM_1, &hdr, sizeof(hdr), portMAX_DELAY) != sizeof(hdr)) {continue;}

		packet.header.type = hdr[0];
		packet.header.count = hdr[1];

		packet.header.timestamp =
			hdr[2] |
			(hdr[3] << 8 )|
			(hdr[4] << 16) |
			(hdr[5] << 24);

		//---------------------------------------------------------
		// Determine payload size
		//---------------------------------------------------------
		size_t payloadLen = getPayloadLength(packet.header.type, packet.header.count);

		if(payloadLen == 0) {
			ESP_LOGW(pcTaskGetName(NULL), "Invalid Payload Length");
			continue;
		}

		//---------------------------------------------------------
		// Read Payload
		//---------------------------------------------------------
		if(uart_read_bytes(UART_NUM_1, packet.payload, payloadLen, pdMS_TO_TICKS(100)) != payloadLen) {continue;}

		//---------------------------------------------------------
		// Read CRC
		//---------------------------------------------------------
		uint8_t rxCRC;
		if(uart_read_bytes(UART_NUM_1, &rxCRC, 1, pdMS_TO_TICKS(100)) != 1){continue;}

		//---------------------------------------------------------
		// Verify CRC
		//---------------------------------------------------------
		uint8_t crcBuffer[6 + MAX_PAYLOAD_SIZE];

		memcpy(crcBuffer, hdr, 6);
		memcpy(crcBuffer + 6, packet.payload, payloadLen);

		uint8_t calcCRC = crc8(crcBuffer, 6 + payloadLen);

		if(calcCRC != rxCRC){
			ESP_LOGW(pcTaskGetName(NULL), "CRC Mismatch rx=%02X calc=%02X", rxCRC, calcCRC);
			continue;
		}

		//---------------------------------------------------------
		// Push Complete Package
		//---------------------------------------------------------
		size_t packetSize = sizeof(PacketHeader) + payloadLen;

		size_t sent = xMessageBufferSend(
			xMessageBufferRx,
			&packet,
			packetSize,
			pdMS_TO_TICKS(100)
		);

		if(sent != packetSize){
			ESP_LOGE(pcTaskGetName(NULL), "Message Buffer Full");
		} else {
			ESP_LOGI(pcTaskGetName(NULL), "Packet Type=%u Count=%u Timestap=%lu,", packet.header.type, packet.header.count, packet.header.timestamp);
			ESP_LOG_BUFFER_HEX("uart", packet.payload, payloadLen); 
		}

	}


	// Stop connection
	ESP_LOGI(pcTaskGetName(NULL), "Task Delete");
	vTaskDelete(NULL);
}
