/*
 * PulsOxHandler.h
 *
 *  Created on: 27 Apr 2026
 *      Author: Lucian
 */
//TODO: implement
#ifndef APP_SPULSOX_PULSOXHANDLER_H_
#define APP_SPULSOX_PULSOXHANDLER_H_
#include "FreeRTOS.h"
#include "SensorHandler/SensorHandlerConfig.h"
#include "max3010x.h"
#include "spo2_algorithm.h"

class PulsOxHandler {
public:
	PulsOxHandler();
	virtual ~PulsOxHandler();
	osStatus_t init(SpO2Config cfg);

	void run();

private:
	osMessageQueueId_t mQueue = nullptr;
	TaskHandle_t mTaskHandle = nullptr;

	const static uint8_t BUFFER_LEN = 100;

	MAX3010x* 	mMAX3010x;
	uint32_t irBuffer[BUFFER_LEN]; //infrared LED sensor data
	uint32_t redBuffer[BUFFER_LEN];  //red LED sensor data
};

#endif /* APP_SPULSOX_PULSOXHANDLER_H_ */
