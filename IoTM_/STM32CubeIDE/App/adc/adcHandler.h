/*
 * adcHandler.h
 *
 *  Created on: 27 Apr 2026
 *      Author: Lucian
 */
//TODO: implement
#ifndef APP_ADC_ADCHANDLER_H_
#define APP_ADC_ADCHANDLER_H_
#include "FreeRTOS.h"
#include "SensorHandler/SensorHandlerConfig.h"
#include "adc.h"

class adcHandler {
public:
	adcHandler();
	virtual ~adcHandler();
	BaseType_t init(adcConfig config);

	// Interupt callbacks:
	void adcErrorCallback(ADC_HandleTypeDef *hadc);
	void adcConcCpltCallback(ADC_HandleTypeDef *hadc);

	osMessageQueueId_t getQueue();

	// Task Loop:
	void run();

private:
	adcConfig mConfig;
	AdcDma *mAdc = nullptr;
	AdcChannel *mAdcChannel1 = nullptr;
	osMessageQueueId_t mQueue = nullptr;
	TaskHandle_t mTaskHandle = nullptr;
};

#endif /* APP_ADC_ADCHANDLER_H_ */
