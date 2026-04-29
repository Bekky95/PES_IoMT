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
	static void StartAdcSensors(void *argument);

private:
	void run();
	AdcDma* mAdc = nullptr;
	AdcChannel* 		mAdcChannel1;
	osMessageQueueId_t	mQueue;
};

#endif /* APP_ADC_ADCHANDLER_H_ */
