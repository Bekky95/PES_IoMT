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

//TODO: maybe shrink this to only hold one adc sensor value if size it too latge
typedef struct __attribute__((packed)) {
    uint8_t   startByte;
    uint8_t   sensorType;
    uint32_t  timestamp_ms;   // first sample's timestamp
    uint8_t   numSamples;
    union {
        struct {
            float emg[TX_BATCH_SIZE];
            float eeg[TX_BATCH_SIZE];
            float ekg[TX_BATCH_SIZE];
        } adcSamples;
        MAX3010x_Data spo2Samples[TX_BATCH_SIZE];
    };
} TxPacket;

typedef struct {
    float         emg[TX_BATCH_SIZE];
    float         eeg[TX_BATCH_SIZE];
    float         ekg[TX_BATCH_SIZE];
    MAX3010x_Data spo2[TX_BATCH_SIZE];
    uint8_t       count;
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
	TaskHandle_t mTaskHandle = nullptr;
};

#endif /* APP_UART_UARTHANDLER_H_ */
