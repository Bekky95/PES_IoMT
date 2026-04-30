/*
 * adcHandler.cpp
 *
 *  Created on: 27 Apr 2026
 *      Author: Lucian
 */

#include <adc/adcHandler.h>

// Notify Bitmask to identify which interrupt was triggered. Bzw which flag is set.
namespace ADC_NotifyBits {
   constexpr uint32_t ADC_DMA_COMPLETE = (1 << 0);
   constexpr uint32_t ADC_ERROR_CALLBACK = (1 << 1);
}

extern osThreadId_t tSensorHandlerHandle;

static adcHandler adcHandlerInstance;
extern "C" adcHandler* adcHandlerGetInstance() {
	return &adcHandlerInstance;
}

extern "C" void ADCHandler_TaskEntry(void* arg) {
	static_cast<adcHandler*>(arg)->run();
}

extern "C" BaseType_t adcInit(adcConfig cfg) {
	return adcHandlerInstance.init(cfg);
}
// ADC conversion complete interrupt callback
extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	adcHandlerInstance.adcConcCpltCallback(hadc);
}

// ADC error callback TODO
extern "C" void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
	adcHandlerInstance.adcErrorCallback(hadc);
}


adcHandler::adcHandler() {
	// TODO Auto-generated constructor stub

}

adcHandler::~adcHandler() {
	// TODO Auto-generated destructor stub
	delete mAdc;
}

BaseType_t adcHandler::init(adcConfig config) {
	BaseType_t stat = pdTRUE;
	mConfig = config;
    if (config.adc) {
        mAdc = new AdcDma(config.adc, config.adcChannelCount);
        mAdcChannel1 = mAdc->registerChannel(0);
    } else if(config.queue) {
    	mQueue = config.queue;
    } else {
    	vTaskSuspend(nullptr);
    	stat = pdFALSE;
    }
    return stat;
}
void adcHandler::adcErrorCallback(ADC_HandleTypeDef* hadc) {
	/// ISR!!! dont call any blocking functions and keep it quick
	if(hadc != mConfig.adc) return;
	//must be set to false, vTaskNotifyGiveFromISR() will set to true if it unblocks tasks
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(mTaskHandle, ADC_NotifyBits::ADC_ERROR_CALLBACK, eSetBits, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void adcHandler::adcConcCpltCallback(ADC_HandleTypeDef* hadc) {
	// ISR!!! dont call any blocking functions and keep it quick
	if(hadc != mConfig.adc) return;
	//must be set to false, vTaskNotifyGiveFromISR() will set to true if it unblocks tasks
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(mTaskHandle, ADC_NotifyBits::ADC_DMA_COMPLETE, eSetBits, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

osMessageQueueId_t adcHandler::getQueue() {
	return mQueue;
}


void adcHandler::run() {
	//T
	mTaskHandle = xTaskGetCurrentTaskHandle();
	uint32_t bits = 0;

	// Start the ADC
	auto stat = mAdc->start();
	configASSERT(stat == HAL_OK);

	while(1) {
		// 0: dont clear bits on entry
		// 0xFFFFFFFF: clear bits on exit
		xTaskNotifyWait(0, 0xFFFFFFFF, &bits, portMAX_DELAY);

		if (bits & ADC_NotifyBits::ADC_DMA_COMPLETE){
			float adcData = mAdcChannel1->getVoltValue();
			osMessageQueuePut(mQueue, &adcData, 0, 0);
			//TODO: check initalization of senorhandler task before calling this? Needed?
			xTaskNotify(static_cast<TaskHandle_t>(tSensorHandlerHandle), SENSOR_HANDLER_NOTIFYBITS_NEW_ADC_DATA, eSetBits);
			//TODO: notify Sensor Handler about new data here!!
		}
		if(bits & ADC_NotifyBits::ADC_ERROR_CALLBACK) {
			// TODO: ensure this works??
			mAdc->stop();
			mAdc->start();
		}
	}
}
