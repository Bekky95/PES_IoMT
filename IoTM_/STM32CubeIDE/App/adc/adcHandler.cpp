/*
 * adcHandler.cpp
 *
 *  Created on: 27 Apr 2026
 *      Author: Lucian
 */

#include <adc/adcHandler.h>
extern uint8_t UI_READY;

// Notify Bitmask to identify which interrupt was triggered. Bzw which flag is set.
namespace ADC_NotifyBits {
   constexpr uint32_t ADC_DMA_COMPLETE = (1 << 0);
   constexpr uint32_t ADC_ERROR_CALLBACK = (1 << 1);
}

extern "C" void SensorHandler_NotifyADC();

extern osThreadId_t tSensorHandlerHandle;

static adcHandler adcHandlerInstance;
extern "C" void* adcHandlerGetInstance() {
	return static_cast<void*>(&adcHandlerInstance);
}

extern "C" void ADCHandler_TaskEntry(void* arg) {
	static_cast<adcHandler*>(arg)->run();
}

extern "C" osStatus_t adcInit(adcConfig cfg) {
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
	//delete mAdc;
}

osStatus_t adcHandler::init(adcConfig config) {
	//DEBUG: volatile uint32_t adcInstance = (uint32_t)config.adc->Instance;
	osStatus_t stat = osOK;
	mConfig = config;
	//TODO clean up
    if (config.adc) {
        mAdc = AdcDma(config.adc, config.adcChannelCount);
    }  else {
    	stat = osError;
    }
    if(config.queue) {
    	mQueue = config.queue;
    } else {
    	stat = osError;
    }
    return stat;
}
void adcHandler::adcErrorCallback(ADC_HandleTypeDef* hadc) {
	/// ISR!!! dont call any blocking functions and keep it quick
	if(hadc != mConfig.adc) return;
	if(!mTaskHandle) return;
	//must be set to false, vTaskNotifyGiveFromISR() will set to true if it unblocks tasks
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(mTaskHandle, ADC_NotifyBits::ADC_ERROR_CALLBACK, eSetBits, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void adcHandler::adcConcCpltCallback(ADC_HandleTypeDef* hadc) {
	// ISR!!! dont call any blocking functions and keep it quick
	if(hadc != mConfig.adc) return;
	if(!mTaskHandle) return;
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
	while(!UI_READY){
		osDelay(50);
	}

	// if any of the adc senors are configed set up adc
	if(USE_EEG_SENSOR || USE_EKG_SENSOR || USE_EMG_SENSOR) {
		// init the adc
		HAL_StatusTypeDef stat = mAdc.start();
		configASSERT(stat == HAL_OK);
	}

	while(1) {
		// 0: dont clear bits on entry
		// 0xFFFFFFFF: clear bits on exit
		xTaskNotifyWait(0, 0xFFFFFFFF, &bits, pdMS_TO_TICKS(100));

		if (bits & ADC_NotifyBits::ADC_DMA_COMPLETE){
			AdcSnapshot snapshot;
			snapshot.timestamp_ms = osKernelGetTickCount();

			for(uint8_t i = 0; i < ADC_CH_COUNT; i++) {
				snapshot.values[i] = mAdc.GetChValVolt(i);
			}
			osMessageQueuePut(mQueue, &snapshot, 0, 0);
			//TODO: check initalization of senorhandler task before calling this? Needed?
			SensorHandler_NotifyADC();
		}
		if(bits & ADC_NotifyBits::ADC_ERROR_CALLBACK) {
			__BKPT();
			// TODO: ensure this works??
			mAdc.stop();
			mAdc.start();
		}
	}
}

uint16_t* adcHandler::getBuffer(){
	return mAdc.getValues();
}
