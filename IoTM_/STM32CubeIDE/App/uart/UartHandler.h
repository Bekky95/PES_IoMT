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

// Frame format (for ESP32 UART → MQTT parser)
// [0xAA][0x55][type:1][count:1][ts_ms:4 LE][payload][crc8:1]
//

#define TASK_QUEUE_TIMEOUT 10  // ms to wait on queue

#define TX_BATCH_SIZE        10
#define UART_SYNC1           0xAAU
#define UART_SYNC2           0x55U
#define MAX_BYTES_PER_SAMPLE 10U                         /* MAX3010x largest */
#define MAX_FRAME_SIZE       (8U + TX_BATCH_SIZE * MAX_BYTES_PER_SAMPLE)

typedef struct {
    union {
        MAX3010x_Data maxData[TX_BATCH_SIZE];
        AdcSnapshot   adcData[TX_BATCH_SIZE];  /* ADC_COMBINED: all 3 ch    */
        uint16_t      singleAdc[TX_BATCH_SIZE];/* EMG/EEG/EKG: one channel  */
    };
} BatchBuffer;

class UartHandler {
public:
	UartHandler();
	virtual ~UartHandler();

	osStatus_t init(uartConfig config);

	void onTxComplete(UART_HandleTypeDef *huart);
	void flushBatch(BatchBuffer *batch, uint8_t count,
            SensorType type, uint32_t startTs);
	void run();
private:
	UART_HandleTypeDef *mUart;
	osMessageQueueId_t mQueue;
	SemaphoreHandle_t  mTxDoneSem;
	TaskHandle_t mTaskHandle = nullptr;
};

#endif /* APP_UART_UARTHANDLER_H_ */
