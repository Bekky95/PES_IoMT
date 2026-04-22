/*
 * SensorHandler.hpp
 *
 *  Created on: 12 Apr 2026
 *      Author: Lucian
 */

#ifndef APP_SENSORHANDLER_SENSORHANDLER_H_
#define APP_SENSORHANDLER_SENSORHANDLER_H_

#include "App/adc/adc.h"
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
	SensorHandler() = default;
	SensorHandler(const SensorHandler&) = delete;
	SensorHandler(const SensorHandlerConfig* config);
	~SensorHandler();

    void init(const SensorHandlerConfig* config);  // called once inside start()
    static void taskEntry(void* pv);
    void taskLoop();


	void onAdcData(uint8_t channel, const uint16_t* data, uint8_t size);
	bool readI2C();

	static SensorHandler*	sInstance;

	SensorHandlerConfig mConfig;
	AdcDma*		mAdc;
	AdcChannel* 		mAdcChannel1;
	osEventFlagsId_t	mflags;
	QueueHandle_t		mUIQueue = nullptr	;
	uint8_t				mI2cBuf[4]					 = {};
	TaskHandle_t		mTaskHandle;
	SemaphoreHandle_t	mUiSem = nullptr;
	bool				mRunning = false;
};

#endif /* APP_SENSORHANDLER_SENSORHANDLER_H_ */
