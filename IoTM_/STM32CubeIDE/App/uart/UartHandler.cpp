/*
 * UartHandler.cpp
 *
 *  Created on: 10 May 2026
 *      Author: Lucian
 */

#include <uart/UartHandler.h>

static UartHandler uartHandlerInstance;
extern uint8_t UI_READY;


extern "C" void UartHandler_TaskEntry(void* arg) {
	static_cast<UartHandler*>(arg)->run();
}


extern "C" void* UartHandlerGetInstance(void) {
	return static_cast<void*>(&uartHandlerInstance);
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {

	uartHandlerInstance.onTxComplete(huart);
}

static void accumulateSample(BatchBuffer* batch, uint8_t index, const SensorData* data)
{
    switch (data->type)
    {
        case SensorType::EMG:  batch->raw[index] = data->EmgData;  break;
        case SensorType::EEG:  batch->raw[index] = data->EegData;  break;
        case SensorType::EKG:  batch->raw[index] = data->EkgData;  break;
        case SensorType::MAX1030x: batch->spo2[index] = data->SpO2Data; break;
        default: break;
    }
}

UartHandler::UartHandler() {
	// TODO Auto-generated constructor stub

}

UartHandler::~UartHandler() {
	// TODO Auto-generated destructor stub
}

void UartHandler::onTxComplete(UART_HandleTypeDef *huart) {
	// release data semaphore after txComplete
	if (huart == mUart) {
		osSemaphoreRelease(mTxDoneSem);
	}
}

void UartHandler::flushBatch(const BatchBuffer*batch, uint8_t count, SensorType type,
		uint32_t firstTimestamp) {
	if (count == 0) {
		return;
	}
	// Wait for previous DMA TX to finish before starting a new transfer
	osSemaphoreAcquire(mTxDoneSem, osWaitForever);

	static TxPacket pkt;
	pkt.startByte = 0xAA;
	pkt.sensorType = (uint8_t) type;
	pkt.timestamp_ms = firstTimestamp;
	pkt.numSamples = count;
	uint16_t payloadSize;

	if(type == SensorType::MAX1030x) {
        memcpy(pkt.spo2Samples, batch->spo2, count * sizeof(MAX3010x_Data));
        payloadSize = count * sizeof(MAX3010x_Data);
	}
    else
    {
        memcpy(pkt.rawSamples, batch->raw, count * sizeof(uint16_t));
        payloadSize = count * sizeof(uint16_t);
    }
	//TODO: see if crc is needed?
	// Calculate length of data packet;
	uint16_t len = offsetof(TxPacket, rawSamples) + payloadSize;
			//TODO add if using crc:+ sizeof(uint16_t);
	// Transmit data Packet, semaphore freed in TX complete callback
	HAL_UART_Transmit_DMA(mUart, (uint8_t*) &pkt, len);
}

osStatus_t UartHandler::init(uartConfig config) {
	//TODO: clean up
	mTxDoneSem = osSemaphoreNew(1, 1, NULL);  // starts available
	mUart = config.uart;
	mQueue = config.queue;
	osStatus_t stat = osOK;
	return stat;
}

void UartHandler::run() {
	SensorData incoming;
	BatchBuffer batch;
	uint8_t batchCount = 0;
	SensorType batchType = SENSOR_NONE;
	uint32_t batchStartTs = 0;

	while (1) {
		while(!UI_READY) {
			osDelay(50);
		}
		osStatus_t status = osMessageQueueGet(mQueue, &incoming, NULL,
		TASK_QUEUE_TIMEOUT);

		if (status == osOK) {
			// new data with different sensor type
			if (batchCount > 0 && incoming.type != batchType) {
				flushBatch(&batch, batchCount, batchType, batchStartTs);
				batchCount = 0;
			}

			// Start a new batch
			if(batchCount == 0){
                batchType    = incoming.type;
                batchStartTs = incoming.timestamp_ms;
			}

			// Fill batch
			accumulateSample(&batch, batchCount++, &incoming);

			if(batchCount >= TX_BATCH_SIZE) {
				flushBatch(&batch, batchCount, batchType, batchStartTs);
				batchCount = 0;
			}
		}
		else if (status == osErrorTimeout) {
			if(batchCount > 0 ) {
				flushBatch(&batch, batchCount, batchType, batchStartTs);
				batchCount = 0;
			}
		}
	}
}

