/*
 * PulsOxHandler.h
 *
 *  Created on: 27 Apr 2026
 *      Author: Lucian
 */
//TODO: implement
#ifndef APP_SPULSOX_PULSOXHANDLER_H_
#define APP_SPULSOX_PULSOXHANDLER_H_
#include "Config.h"
#include "FreeRTOS.h"
#include "max3010x.h"
#include "spo2_algorithm.h"
#include "SensorHandler/SensorHandler.h"
#include "heartRate.h"

struct HRFilterState
{
    float dc = 0.0f;

    float lp_prev = 0.0f;

    float envelope = 0.0f;

    float bpmFiltered = 0.0f;

    uint32_t sampleIndex = 0;
    uint32_t lastPeakSample = 0;

    float prev2 = 0.0f;
    float prev1 = 0.0f;

    bool initialized = false;
};


class PulsOxHandler {
public:
	PulsOxHandler();
	virtual ~PulsOxHandler();
	osStatus_t init(SpO2Config cfg);
	void errHandler(I2C_HandleTypeDef* hi2c);
	void run();
	bool processHRSample(uint32_t irSample,
	                                    float& bpmOut);

private:
	osMessageQueueId_t mQueue = nullptr;
	TaskHandle_t mTaskHandle = nullptr;

	//const static uint8_t BUFFER_LEN = 100;

	MAX3010x mMAX3010x;
	uint32_t irBuffer[BUFFER_SIZE]; //infrared LED sensor data
	uint32_t redBuffer[BUFFER_SIZE];  //red LED sensor data
	uint8_t  hr_average[8];
	TickType_t lastBeat = 0;
	HRFilterState mHRState;
};

#endif /* APP_SPULSOX_PULSOXHANDLER_H_ */
