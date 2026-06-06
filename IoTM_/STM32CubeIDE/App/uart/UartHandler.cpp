/*
 * UartHandler.cpp
 *
 *  Created on: 10 May 2026
 *      Author: Lucian
 */

#include <uart/UartHandler.h>

static UartHandler uartHandlerInstance;
static TaskHandle_t uartTaskHandle = nullptr;
extern uint8_t UI_READY;

extern "C" void UartHandler_TaskEntry(void *arg) {
	static_cast<UartHandler*>(arg)->run();
}

extern "C" void* UartHandlerGetInstance(void) {
	return static_cast<void*>(&uartHandlerInstance);
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {

	uartHandlerInstance.onTxComplete(huart);
}

extern "C" osStatus_t uartInit(uartConfig cfg) {
	return uartHandlerInstance.init(cfg);
}
extern "C" void notify_UartTask() {
	if (uartTaskHandle != nullptr) {
		xTaskNotify(uartTaskHandle, UART_HANDLER_NEW_TX_DATA, eSetBits);
	}
}

/* -------- CRC-8/DVB-S2 -------- */
static uint8_t crc8(const uint8_t *buf, size_t len) {
	uint8_t crc = 0x00;
	while (len--) {
		crc ^= *buf++;
		for (int i = 0; i < 8; i++)
			crc = (crc & 0x80) ?
					(uint8_t) ((crc << 1) ^ 0x07u) : (uint8_t) (crc << 1);
	}
	return crc;
}

static void accumulateSample(BatchBuffer *batch, uint8_t index,
		const SensorData *data) {
	switch (data->type) {
	case SensorType::ADC_COMBINED:
		batch->adcData[index].emgAvg = data->AdcData.emgAvg;
		batch->adcData[index].eegAvg = data->AdcData.eegAvg;
		batch->adcData[index].ekgAvg = data->AdcData.ekgAvg;
		break;
	case SensorType::MAX1030x:
		batch->maxData[index] = data->SpO2Data;
		break;
	default:
		break;
	}
}

UartHandler::UartHandler() {
	// TODO Auto-generated constructor stub

}

UartHandler::~UartHandler() {
	// TODO Auto-generated destructor stub
}

void UartHandler::onTxComplete(UART_HandleTypeDef *huart) {
	if (huart == mUart) {
		BaseType_t higherPriorityWoken = pdFALSE;
		xSemaphoreGiveFromISR(mTxDoneSem, &higherPriorityWoken);
		portYIELD_FROM_ISR(higherPriorityWoken);
	}
}

void UartHandler::flushBatch(BatchBuffer *batch, uint8_t count, SensorType type,
		uint32_t startTs) {
	if (count == 0 || type == SENSOR_NONE) {
		return;
	}

	uint8_t frame[MAX_FRAME_SIZE];
	uint8_t *p = frame;

	// Uart packet header:
	// Sync bytes:
	*p++ = UART_SYNC1;
	*p++ = UART_SYNC2;
	// Data Type:
	*p++ = (uint8_t) type;
	// Data count:
	*p++ = count;
	// Time Stamp:
	*p++ = (uint8_t) (startTs >> 0);
	*p++ = (uint8_t) (startTs >> 8);
	*p++ = (uint8_t) (startTs >> 16);
	*p++ = (uint8_t) (startTs >> 24);

	// Manual Payload loading to avoid struct-padding
	for (uint8_t i = 0; i < count; i++) {
		switch (type) {
		case ADC_COMBINED:
			*p++ = (uint8_t) (batch->adcData[i].emgAvg & 0xFF);
			*p++ = (uint8_t) (batch->adcData[i].emgAvg >> 8);
			*p++ = (uint8_t) (batch->adcData[i].eegAvg & 0xFF);
			*p++ = (uint8_t) (batch->adcData[i].eegAvg >> 8);
			*p++ = (uint8_t) (batch->adcData[i].ekgAvg & 0xFF);
			*p++ = (uint8_t) (batch->adcData[i].ekgAvg >> 8);
			break;

		case EMG:
		case EEG:
		case EKG:
			*p++ = (uint8_t) (batch->singleAdc[i] & 0xFF);
			*p++ = (uint8_t) (batch->singleAdc[i] >> 8);
			break;

		case MAX1030x:{
			int32_t spo2 = batch->maxData[i].spo2;
			int32_t hr = batch->maxData[i].heartRate;
			*p++ = (uint8_t) (spo2 >> 0);
			*p++ = (uint8_t) (spo2 >> 8);
			*p++ = (uint8_t) (spo2 >> 16);
			*p++ = (uint8_t) (spo2 >> 24);
			*p++ = (uint8_t) batch->maxData[i].validSPO2;
			*p++ = (uint8_t) (hr >> 0);
			*p++ = (uint8_t) (hr >> 8);
			*p++ = (uint8_t) (hr >> 16);
			*p++ = (uint8_t) (hr >> 24);
			*p++ = (uint8_t) batch->maxData[i].validHeartRate;
			break;
		}
		case MAX_Sp02:
		case MAX_HR:
		default:
			break;
		}

	}
	/* CRC over everything after the sync bytes */
	*p = crc8(frame + 2, (size_t) (p - (frame + 2)));
	p++;

	const uint16_t frameLen = (uint16_t) (p - frame);

	//TODO the dma transfer doesnt seem to trigger the callback which never releases the semaphore, fix if needed
	//Aquire the Tx Semaphore
//	xSemaphoreTake(mTxDoneSem, portMAX_DELAY);
//	memcpy(sTxBuf, frame, frameLen);
//	HAL_UART_Transmit_DMA(mUart, sTxBuf, frameLen);
	/* Transmit — swap for HAL_UART_Transmit_DMA to stop blocking here */
	HAL_UART_Transmit(mUart, frame, (uint16_t) (p - frame),
	HAL_MAX_DELAY);
}

osStatus_t UartHandler::init(uartConfig config) {
	//TODO: clean up
	mTxDoneSem = xSemaphoreCreateBinary();
	xSemaphoreGive(mTxDoneSem);
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
	uint32_t bits = 0;

	mTaskHandle = xTaskGetCurrentTaskHandle();
	uartTaskHandle = xTaskGetCurrentTaskHandle();

	while (1) {

		xTaskNotifyWait(0, 0xFFFFFFFF, &bits, pdMS_TO_TICKS(20));

		while (osMessageQueueGet(mQueue, &incoming, nullptr, 0) == osOK) {

			/* Type changed mid-stream → flush the current batch first */
			if (batchCount > 0 && incoming.type != batchType) {
				flushBatch(&batch, batchCount, batchType, batchStartTs);
				batchCount = 0;
			}

			if (batchCount == 0) {
				batchType = incoming.type;
				batchStartTs = incoming.timestamp_ms;
			}

			accumulateSample(&batch, batchCount++, &incoming);

			/* Full batch → ship it immediately */
			if (batchCount >= TX_BATCH_SIZE) {
				flushBatch(&batch, batchCount, batchType, batchStartTs);
				batchCount = 0;
			}
		}


	}
}
