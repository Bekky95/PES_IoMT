/*
 * UartHandler.h
 *
 *  Created on: 10 May 2026
 *      Author: Lucian
 */

#ifndef APP_UART_UARTHANDLER_H_
#define APP_UART_UARTHANDLER_H_
#include "Config.h"
#include "cmsis_os2.h"
#include <cstring>

#define TX_BATCH_SIZE        50
#define TASK_QUEUE_TIMEOUT 10  // ms to wait on queue

typedef struct __attribute__((packed)) {
    uint8_t   startByte;
    uint8_t   sensorType;
    uint32_t  timestamp_ms;   // first sample's timestamp
    uint8_t   numSamples;
    union {
        float      rawSamples[TX_BATCH_SIZE];   // EMG / EEG / EKG
        MAX3010x_Data spo2Samples[TX_BATCH_SIZE];  // SpO2
    };
    //uint16_t  crc;
} TxPacket;

typedef union {
    uint16_t      raw[TX_BATCH_SIZE];
    MAX3010x_Data spo2[TX_BATCH_SIZE];
} BatchBuffer;

class UartHandler {
public:
	UartHandler();
	virtual ~UartHandler();

	osStatus_t init(uartConfig config);

	void onTxComplete(UART_HandleTypeDef *huart);
	void flushBatch(const BatchBuffer* batch, uint8_t count, SensorType type,
			uint32_t firstTimestamp);
	void run();
private:
	UART_HandleTypeDef *mUart;
	osMessageQueueId_t mQueue;
	osSemaphoreId_t mTxDoneSem;
};

#endif /* APP_UART_UARTHANDLER_H_ */
