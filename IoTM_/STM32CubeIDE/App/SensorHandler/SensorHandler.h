/*
 * SensorHandler.hpp
 *
 *  Created on: 12 Apr 2026
 *      Author: Lucian
 */

#ifndef APP_SENSORHANDLER_SENSORHANDLER_H_
#define APP_SENSORHANDLER_SENSORHANDLER_H_

#include "../../STM32CubeIDE/App/adc/adc.h"
#include "SensorHandlerConfig.h"
#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>

class SensorHandler {
public:

	explicit SensorHandler(const SensorHandlerConfig* config);
	~SensorHandler();

	SensorHandler(const SensorHandler&) = delete;
	SensorHandler& operator=(const SensorHandler) = delete;

	static void start(SensorHandlerConfig* config, const osThreadAttr_t* attr);
	void stop();

	static void taskEntry(void* pv);
private:
	void taskLoop();
	void onAdcData(uint8_t channel, const uint16_t* data, uint8_t size);
	bool readI2C();

	SensorHandlerConfig mConfig;

	SemaphoreHandle_t	mAdcMutex 					 = nullptr;

	QueueHandle_t		mUIQueue					 = nullptr;

	uint8_t				mI2cBuf[4]					 = {};

	TaskHandle_t		mTaskHandle				 	 = nullptr;
	bool				mRunning					 = false;
};

#endif /* APP_SENSORHANDLER_SENSORHANDLER_H_ */
