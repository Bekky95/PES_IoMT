/*
 * PulsOxHandler.cpp
 *
 *  Created on: 27 Apr 2026
 *      Author: Lucian
 */

#include <sPulsOx/PulsOxHandler.h>

// TODO maybe protect this in a giver function
extern osThreadId_t tSensorHandlerHandle;

static PulsOxHandler pulsOxHandlerInstance;
extern "C" void* pulsOxHandlerGetInstance() {
	return static_cast<void*>(&pulsOxHandlerInstance);
}

extern "C" void PulsOxHandler_TaskEntry(void *arg) {
	static_cast<PulsOxHandler*>(arg)->run();
}
extern "C" osStatus_t sP02Init(SpO2Config config) {
	return pulsOxHandlerInstance.init(config);
}

extern "C" void SensorHandler_NotifyMAX();

osStatus_t PulsOxHandler::init(SpO2Config cfg) {
	mMAX3010x = MAX3010x(cfg.hi2c);
	mQueue = cfg.queue;

	// Init sensor
	osStatus_t status = (osStatus_t) mMAX3010x.init();

	// Init failed return
	// TODO add error handling/logging
	if (status != osOK) {
		vTaskSuspend(nullptr);
		return status;
	}

	//Setup Sensor
	uint8_t ledBrightness = 60; //Options: 0=Off to 255=50mA
	uint8_t sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
	uint8_t ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
	uint8_t sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
	int pulseWidth = 411; //Options: 69, 118, 215, 411
	int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

	mMAX3010x.setup(ledBrightness, sampleAverage, ledMode, sampleRate,
			pulseWidth, adcRange);

	return status;
}
PulsOxHandler::PulsOxHandler() {
	// TODO Auto-generated constructor stub

}

PulsOxHandler::~PulsOxHandler() {
	// TODO Auto-generated destructor stub
}

void PulsOxHandler::run() {

	mTaskHandle = xTaskGetCurrentTaskHandle();
	uint32_t bits = 0;

	MAX3010x_Data data = { };

	// Collect initial 100 samples
	for (uint8_t i = 0; i < BUFFER_LEN; i++) {

		// Block in small yields until new data is ready
		while (!mMAX3010x.available()) {
			mMAX3010x.check();
			osDelay(0.5);
		}

		redBuffer[i] = mMAX3010x.getRed();
		irBuffer[i] = mMAX3010x.getIR();
		mMAX3010x.nextSample();
	}
	// Initial HR + SpO2 calculation on first 100 samples
	maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_LEN, redBuffer,
			(int32_t*) &data.spo2, (int8_t*) &data.validSPO2,
			(int32_t*) &data.heartRate, (int8_t*) &data.validHeartRate);

	// main loop
	while (USE_SP02_SENSOR) {
		// Shift last 75 samples down, discarding oldest 25
		for (uint8_t i = 25; i < 100; i++) {
			redBuffer[i - 25] = redBuffer[i];
			irBuffer[i - 25] = irBuffer[i];
		}

		// Collect 25 fresh samples into the top of the buffer
		for (uint8_t i = 75; i < 100; i++) {
			while (!mMAX3010x.available()) {
				mMAX3010x.check();
				osDelay(1);
			}

			redBuffer[i] = mMAX3010x.getRed();
			irBuffer[i] = mMAX3010x.getIR();
			mMAX3010x.nextSample();
		}

		// Recalculate
		maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_LEN, redBuffer,
				(int32_t*) &data.spo2, (int8_t*) &data.validSPO2,
				(int32_t*) &data.heartRate, (int8_t*) &data.validHeartRate);

		//TODO send data in queue to sensor handler and set flags
		osMessageQueuePut(mQueue, &data, 0, 0);
		SensorHandler_NotifyMAX();
	}

	// Should never get h6re
	vTaskSuspend(nullptr);
}

