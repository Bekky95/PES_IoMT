/*
 * adcHandler.cpp
 *
 *  Created on: 27 Apr 2026
 *      Author: Lucian
 */

#include <adc/adcHandler.h>

static adcHandler adcHandlerInstance;
adcHandler* adcHandlerGetInstance() { return &adcHandlerInstance;}
extern "C" void ADCHandler_TaskEntry(void* arg) { static_cast<adcHandler*>(arg)->run(); }

extern "C" BaseType_t adcInit(adcConfig cfg) {
	return adcHandlerInstance.init(cfg);
}

adcHandler::adcHandler() {
	// TODO Auto-generated constructor stub

}

adcHandler::~adcHandler() {
	// TODO Auto-generated destructor stub
}

BaseType_t adcHandler::init(adcConfig config) {
	BaseType_t stat = pdTRUE;
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

void run() {
	//TODO: define task loop
}
