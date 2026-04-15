/*
 * SensorHandler.cpp
 *
 *  Created on: 12 Apr 2026
 *      Author: Lucian
 */

#include <SensorHandler/SensorHandler.h>
#include <cstring>
#include <numeric>

extern "C" void SensorHandler_Start(SensorHandlerConfig* config, const osThreadAttr_t* attr)
{
    SensorHandler::start(config, attr);
}

// Constructor / Destructor

SensorHandler::SensorHandler(const SensorHandlerConfig* config) {
	if(config->hadc) {
		mAdc = new AdcDma(config->hadc, config->adcChannelCount);
		mAdcChannel1 = mAdc->registerChannel(0);
	}
}

SensorHandler::~SensorHandler() {
    this->stop();
}

void SensorHandler::stop() {
	mRunning = false;
}
// Public API
void SensorHandler::start(SensorHandlerConfig* config, const osThreadAttr_t* attr) {


	  static SensorHandler sSensorHandler(config);
	  //TODO: Clean this up
	  osThreadId_t tSensorHandlerHandle;
	  tSensorHandlerHandle = osThreadNew(SensorHandler::taskEntry,
	                                     &sSensorHandler,
	                                     attr);

};

void SensorHandler::taskEntry(void* pv) {
	static_cast<SensorHandler*>(pv)->taskLoop();
}

void SensorHandler::taskLoop() {

	//TODO: fix here, read data from sensors and send to display/mqtt
    while (mRunning) {

        // ── Pace the loop ────────────────────────────────────────────────────
        vTaskDelay(pdMS_TO_TICKS(mConfig.loopPeriodMs));
    }
}
