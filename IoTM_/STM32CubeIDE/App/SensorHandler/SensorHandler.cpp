/*
 * SensorHandler.cpp
 *
 *  Created on: 12 Apr 2026
 *      Author: Lucian
 */

#include <SensorHandler/SensorHandler.h>
#include <cstring>
#include <numeric>
#include <cstdio>

SensorHandler* SensorHandler::sInstance = nullptr;
static osThreadId_t tSensorHandlerHandle;


extern "C" void SensorHandler_Start(SensorHandlerConfig* config, const osThreadAttr_t* attr)
{
    SensorHandler::start(config, attr);
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
void SensorHandler::start(SensorHandlerConfig* config, const osThreadAttr_t* attr) {

	  if(sInstance != nullptr) {
		  return;
	  }

	  sInstance = new SensorHandler();
	  sInstance->init(config);
	  //TODO: Clean this up
	  tSensorHandlerHandle = osThreadNew(SensorHandler::taskEntry,
	                                     sInstance,
	                                     attr);
};

void SensorHandler::init(const SensorHandlerConfig* config) {
    mConfig = *config;
    mRunning = true;

    if (config->hi2c) {
    	mMax3010x = new MAX3010x(config->hi2c);
    }

    mUIQueue = mConfig.uiQueue;
    mAdcQueue = mConfig.adcQueue;
    mUiSem = mConfig.uiSem;
}


const QueueHandle_t SensorHandler::getUIQueue(void) const{
	return mUIQueue;
}

const SemaphoreHandle_t SensorHandler::getUiSemaphore(void) const {
	return mUiSem;
}

void SensorHandler::taskEntry(void* pv) {
	static_cast<SensorHandler*>(pv)->taskLoop();
}

void SensorHandler::taskLoop() {

	mTaskHandle = xTaskGetCurrentTaskHandle();
	const TickType_t ticksToWait = pdMS_TO_TICKS(100);
	configASSERT(mAdc != nullptr);
	configASSERT(mMax3010x != nullptr);

	// init the adc
	auto stat = mAdc->start();
	configASSERT(stat == HAL_OK);

	// init the MAX3010x
	stat = mMax3010x->init();
	configASSERT(stat == HAL_OK);
	//start the MAX3010x sensor with default params
	mMax3010x->setup();

	// Notification bits
	uint32_t bits = 0;

	//TODO: fix here, read data from sensors and send to display/mqtt
    while (mRunning) {
    	printf("test");
    	osStatus_t status = osOK;

    	SensorData data = {};

    	// Wait for notification from other tasks
    	xTaskNotifyWait(0, 0xFFFFFFFF, &bits, portMAX_DELAY);

    	if(bits & SENSOR_HANDLER_NOTIFYBITS_NEW_ADC_DATA) {

    		float adcData = 0;

    		if(osMessageQueueGet(mAdcQueue, &adcData, nullptr, 0) == osOK) {
				// ADC interrupt fired handle by passing data to sources
				// TODO handle more than one adc source
				data.EmgData = adcData;
				// Get semahpore and write data to the UI Queue

    		}
    		else {
    			status = osError;
    		}
    	}
    	// check if MAX3010x has new data for 1ms warning blocking function!
    	if(mMax3010x->safeCheck(pdMS_TO_TICKS(1))) {

    	}

    	// if Status ok send new data to UI
    	// TODO: maybe send it everytime new data is captured so that data is always fresh
    	// TODO: send data as a ID to show which sensor and pointer to data to make handling easier
    	if(status == osOK) {
    		// no need to notify the UI task as it triggers every tick (60Hz??)
    		osMessageQueuePut(mUIQueue, &data, 0, 0);
    	}

    }
    // terminate task if mRunning is set to false
    vTaskDelete(NULL);
}
