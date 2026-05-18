/*
 * SensorHandler.cpp
 *
 *  Created on: 12 Apr 2026
 *      Author: Lucian
 */

#include <SensorHandler/SensorHandler.h>
extern uint8_t UI_READY;
//TODO maybe move
// Maps ADC buffer index → SensorType
static const SensorType ADC_CHANNEL_TYPE[3] = { SensorType::EMG,
		SensorType::EEG, SensorType::EKG, };

SensorHandler *SensorHandler::sInstance = nullptr;
static osThreadId_t tSensorHandlerHandle;

extern "C" void notify_UartTask();

extern "C" void SensorHandler_Start(SensorHandlerConfig *config,
		const osThreadAttr_t *attr) {
	SensorHandler::start(config, attr);
}

extern "C" void SensorHandler_NotifyADC() {
	if (tSensorHandlerHandle != nullptr) {
		xTaskNotify(static_cast<TaskHandle_t>(tSensorHandlerHandle),
				SENSOR_HANDLER_NOTIFYBITS_NEW_ADC_DATA, eSetBits);
	}
}

extern "C" void SensorHandler_NotifyMAX() {
	if (tSensorHandlerHandle != nullptr) {
		xTaskNotify(static_cast<TaskHandle_t>(tSensorHandlerHandle),
				SENSOR_HANDLER_NOTIFYBITS_NEW_MAX_DATA, eSetBits);
	}
}
SensorHandler::~SensorHandler() {
	this->stop();
}

void SensorHandler::stop() {
	mRunning = false;
}

SensorHandler& SensorHandler::instance() {
	configASSERT(sInstance != nullptr);
	return *sInstance;
}
// Public API
void SensorHandler::start(SensorHandlerConfig *config,
		const osThreadAttr_t *attr) {

	if (sInstance != nullptr) {
		return;
	}

	sInstance = new SensorHandler();
	sInstance->init(config);
	//TODO: Clean this up
	tSensorHandlerHandle = osThreadNew(SensorHandler::taskEntry, sInstance,
			attr);
}


void SensorHandler::init(const SensorHandlerConfig *config) {
	mConfig = *config;
	mRunning = true;

	mUIQueue = mConfig.uiQueue;
	mAdcQueue = mConfig.adcQueue;
	mMax3010xQueue = mConfig.max3010xQueue;
	mUartQueue = mConfig.uartQueue;
	mUiSem = mConfig.uiSem;
}

const QueueHandle_t SensorHandler::getUIQueue(void) const {
	return mUIQueue;
}

const SemaphoreHandle_t SensorHandler::getUiSemaphore(void) const {
	return mUiSem;
}

void SensorHandler::taskEntry(void *pv) {
	static_cast<SensorHandler*>(pv)->taskLoop();
}

void SensorHandler::taskLoop() {

	mTaskHandle = xTaskGetCurrentTaskHandle();
	// Notification bits
	uint32_t bits = 0;

	//TODO: fix here, read data from sensors and send to display/mqtt
	while (mRunning) {
		while (!UI_READY) {
			osDelay(50);
		}
		//osStatus_t status = osOK;

		// Wait for notification from other tasks
		xTaskNotifyWait(0, 0xFFFFFFFF, &bits, pdMS_TO_TICKS(100));

		if (bits & SENSOR_HANDLER_NOTIFYBITS_NEW_ADC_DATA) {

			AdcSnapshot adcData;

			// drain queue as there should be more than one value
			while (osMessageQueueGet(mAdcQueue, &adcData, nullptr, 0) == osOK) {
				// ADC interrupt fired handle by passing data to sources

				for (uint8_t i = 0; i < ADC_CH_COUNT; i++) {
					SensorData data = { };
					data.type = ADC_CHANNEL_TYPE[i];
					data.timestamp_ms = adcData.timestamp_ms;

					switch (i) {
					case ADC_CH_EMG:
						data.EmgData = adcData.values[i];
						break;
					case ADC_CH_EEG:
						data.EegData = adcData.values[i];
						break;
					case ADC_CH_EKG:
						data.EkgData = adcData.values[i];
						break;
					}
					publishToAll(data);
				}

			}
			// check if MAX3010x has new data for 1ms warning blocking function!
			if (bits & SENSOR_HANDLER_NOTIFYBITS_NEW_MAX_DATA) {
				MAX3010x_Data MAX3010xData;

				// Drain Data
				while (osMessageQueueGet(mMax3010xQueue, &MAX3010xData, nullptr,
						0) == osOK) {
					SensorData data = { };
					data.type = SensorType::MAX1030x;
					data.timestamp_ms = osKernelGetTickCount();
					data.SpO2Data = MAX3010xData;

					publishToAll(data);
				}
			}

		}
	}
	// terminate task if mRunning is set to false
	__BKPT();
	vTaskDelete(NULL);
}
void SensorHandler::publishToAll(SensorData data) {
	//TODO rate limit the sending to UI, find a better fix
	uint32_t lastUISend = 0;
	const uint32_t UI_UPDATE_MS = 33; // ~30fps
	uint32_t now = osKernelGetTickCount();
	if (now - lastUISend >= UI_UPDATE_MS) {
		osStatus_t stat = osOK;
		//uint32_t cnt = osMessageQueueGetCount(mUIQueue);
		if (USE_UI && mUIQueue != nullptr) {
			// No need to notify the UI Task as it triggers every tick (60Hz)
			stat = osMessageQueuePut(mUIQueue, &data, 0, 0);
		}
		if (USE_MQTT) {
			// TODO implement MQTT Task
			stat = osMessageQueuePut(mUartQueue, &data, 0, 0);

			if (stat == osOK) {
				notify_UartTask();
			}
		}
		if (stat != osOK) {
			__BKPT();
		}
		lastUISend = now;
	}

}
