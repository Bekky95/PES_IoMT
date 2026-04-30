/*
 * SensorHandler.hpp
 *
 *  Created on: 12 Apr 2026
 *      Author: Lucian
 */

#ifndef APP_SENSORHANDLER_SENSORHANDLER_H_
#define APP_SENSORHANDLER_SENSORHANDLER_H_

#include "App/adc/adc.h"
#include "App/sPulsOx/max3010x.h"
#include "SensorHandlerConfig.h"
#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>

static constexpr uint32_t ADC_HALF = 0x01;
static constexpr uint32_t ADC_FULL = 0x02;

class SensorHandler {
public:

	static SensorHandler&	instance();
	SensorHandler& operator=(const SensorHandler) = delete;

	static void start(SensorHandlerConfig* config, const osThreadAttr_t* attr);
	void stop();
	void notifyAdc(ADC_HandleTypeDef* hadc);
	const QueueHandle_t getUIQueue(void) const;
	const SemaphoreHandle_t getUiSemaphore(void) const;

private:
	//Constructors and Destructor:
	SensorHandler() = default;
	SensorHandler(const SensorHandler&) = delete;
	SensorHandler(const SensorHandlerConfig* config);
	~SensorHandler();

	// Init and Task Stuff:
    void init(const SensorHandlerConfig* config);  // called once inside start()
    static void taskEntry(void* pv);
    void taskLoop();

    // Data functions:
	void onAdcData(uint8_t channel, const uint16_t* data, uint8_t size);
	bool readI2C();

	// The global Instance
	static SensorHandler*	sInstance;

	// Hardware Handles:
	SensorHandlerConfig mConfig;

	// ADC:
	// TODO: move adc into sensor wrapper classes i.e sEEG(adcChannel1)...
	AdcDma*		mAdc = nullptr;
	AdcChannel* 		mAdcChannel1;

	//MAX3010x HR and SpO2 Sensor:
	MAX3010x*	mMax3010x = nullptr;

	//osEventFlagsId_t	mflags;
	QueueHandle_t		mUIQueue = nullptr	;
	osMessageQueueId_t	mAdcQueue = nullptr;
	TaskHandle_t		mTaskHandle;
	SemaphoreHandle_t	mUiSem = nullptr;
	bool				mRunning = false;
};

#endif /* APP_SENSORHANDLER_SENSORHANDLER_H_ */
