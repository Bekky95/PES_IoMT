/*
 * adcHandler.cpp
 *
 *  Created on: 27 Apr 2026
 *      Author: Lucian
 */

#include <adc/adcHandler.h>

namespace NotifyBits {
   constexpr uint32_t ADC_DMA_COMPLETE = (1 << 0);
   constexpr uint32_t ADC_ERROR_CALLBACK = (1 << 1);
}

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

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	adcHandlerInstance.adcConcCpltCallback(hadc);
}

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
    	stat = pdFALSE;
    }
    return stat;
}
void adcHandler::adcErrorCallback(ADC_HandleTypeDef* hadc) {
	/// ISR!!! dont call any blocking functions and keep it quick

}

void adcHandler::adcConcCpltCallback(ADC_HandleTypeDef* hadc) {
	// ISR!!! dont call any blocking functions and keep it quick
	if(hadc != mConfig.adc) return;
	//must be set to false, vTaskNotifyGiveFromISR() will set to true if it unblocks tasks
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(mTaskHandle, NotifyBits::ADC_DMA_COMPLETE, eSetBits, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void adcHandler::run() {
	//T
	mTaskHandle = xTaskGetCurrentTaskHandle();
	uint32_t bits = 0;

	// 0: dont clear bits on entry
	// 0xFFFFFFFF: clear bits on exit
	xTaskNotifyWait(0, 0xFFFFFFFF, &bits, portMAX_DELAY);

	if (bits & NotifyBits::ADC_DMA_COMPLETE){
		float adcData = mAdcChannel1->getVoltValue();
		osMessageQueuePut(mQueue, &adcData, 0, 0);
		//TODO: notify Sensor Handler about new data here!!
	}

}
