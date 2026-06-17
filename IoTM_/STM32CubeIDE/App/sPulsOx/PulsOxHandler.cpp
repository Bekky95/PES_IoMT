/*
 * PulsOxHandler.cpp
 *
 *  Created on: 27 Apr 2026
 *      Author: Lucian
 */

#include <sPulsOx/PulsOxHandler.h>
extern uint8_t UI_READY;
extern SensorType _activeType;
// TODO maybe protect this in a giver function
extern osThreadId_t tSensorHandlerHandle;
static PulsOxHandler pulsOxHandlerInstance;
extern I2C_HandleTypeDef hi2c1;

const uint16_t NEW_SAMPLES = BUFFER_SIZE / 4;

extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
	pulsOxHandlerInstance.errHandler(hi2c);
}

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

void PulsOxHandler::errHandler(I2C_HandleTypeDef *hi2c) {
	if (hi2c == &hi2c1) {
		static uint32_t i2c = hi2c->ErrorCode;
	}
}
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
	//Setup Sensor
	uint8_t ledBrightness = 60; //Options: 0=Off to 255=50mA
	uint8_t sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
	uint8_t ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
	int sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
	int pulseWidth = 411; //Options: 69, 118, 215, 411
	int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

	// Seperate heart rate stuff
	static const uint8_t RATE_SIZE = 4;
	static uint8_t rates[RATE_SIZE];
	static uint8_t rateSpot = 0;

	static uint32_t lastBeat = 0;

	static float beatsPerMinute = 0;
	static int beatAvg = 0;
	static uint8_t numsamp = 0;

	rateSpot = 0;
	lastBeat = HAL_GetTick();
	memset(rates, 0, sizeof(rates));

	mMAX3010x.setup(ledBrightness, sampleAverage, ledMode, sampleRate,
			pulseWidth, adcRange);

	MAX3010x_Data data = { };

	while (_activeType != SensorType::MAX1030x) {
		osDelay(pdMS_TO_TICKS(10));
	}

	// Collect initial BUFFER_SIZE samples
	for (uint8_t i = 0; i < BUFFER_SIZE; i++) {

		// Block in small yields until new data is ready
		numsamp = mMAX3010x.available();
		while (numsamp == 0) {
			mMAX3010x.check();
			numsamp = mMAX3010x.available();
			osDelay(pdMS_TO_TICKS(35));
		}

		uint32_t red = mMAX3010x.getRed();
		uint32_t ir = mMAX3010x.getIR();

		redBuffer[i] = red;
		irBuffer[i] = ir;
		if (checkForBeat(ir)) {
			uint32_t now = HAL_GetTick();
			uint32_t delta = now - lastBeat;
			lastBeat = now;

			if (delta > 0) {
				beatsPerMinute = 60.0f / (delta / 1000.0f);

				if (beatsPerMinute < 255 && beatsPerMinute > 20) {
					rates[rateSpot++] = (uint8_t) beatsPerMinute;
					rateSpot %= RATE_SIZE;

					int sum = 0;
					for (uint8_t i = 0; i < RATE_SIZE; i++) {
						sum += rates[i];
					}
					beatAvg = sum / RATE_SIZE;
				}
			}
		}
		mMAX3010x.nextSample();
	}

	// Initial HR + SpO2 calculation on first 100 samples
	maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer,
			(int32_t*) &data.spo2, (int8_t*) &data.validSPO2,
			(int32_t*) &data.heartRate, (int8_t*) &data.validHeartRate);
	// main loop
	while (USE_SP02_SENSOR) {
		while (!UI_READY) {
			osDelay(1000);
		}

		xTaskNotifyWait(0, 0xFFFFFFFF, &bits, 0);
		if ((bits & MAX3010x_STOP_FLAG)
				|| (_activeType != SensorType::MAX1030x)) {
			mMAX3010x.shutDown();
			SensorHandler::instance().onStopped();
			vTaskSuspend(NULL);
			mMAX3010x.wakeUp();
			continue;
		}
		// Shift last 75 samples down, discarding oldest 25
		for (uint16_t i = NEW_SAMPLES; i < BUFFER_SIZE; i++) {
			redBuffer[i - NEW_SAMPLES] = redBuffer[i];
			irBuffer[i - NEW_SAMPLES] = irBuffer[i];
		}

		// Collect 25 fresh samples into the top of the buffer
		for (uint16_t i = (BUFFER_SIZE - NEW_SAMPLES); i < BUFFER_SIZE; i++) {
			while (mMAX3010x.available() == 0) {
				mMAX3010x.check();
				osDelay(pdMS_TO_TICKS(35));
			}
			uint32_t red = mMAX3010x.getRed();
			uint32_t ir = mMAX3010x.getIR();

			redBuffer[i] = red;
			irBuffer[i] = ir;

			if (checkForBeat(ir)) {
				uint32_t now = HAL_GetTick();
				uint32_t delta = now - lastBeat;
				lastBeat = now;

				if (delta > 0) {
					beatsPerMinute = 60.0f / (delta / 1000.0f);

					if (beatsPerMinute < 255 && beatsPerMinute > 20) {
						rates[rateSpot++] = (uint8_t) beatsPerMinute;
						rateSpot %= RATE_SIZE;

						int sum = 0;
						for (uint8_t i = 0; i < RATE_SIZE; i++) {
							sum += rates[i];
						}
						beatAvg = sum / RATE_SIZE;
					}
				}
			}

			mMAX3010x.nextSample();
		}

		// Recalculate
		maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer,
				(int32_t*) &data.spo2, (int8_t*) &data.validSPO2,
				(int32_t*) &data.heartRate, (int8_t*) &data.validHeartRate);

		// Copy the newest NEW_SAMPLES raw values into data before queuing
		memcpy(data.irSamples, &irBuffer[BUFFER_SIZE - NEW_SAMPLES],
				NEW_SAMPLES * sizeof(uint32_t));
		memcpy(data.redSamples, &redBuffer[BUFFER_SIZE - NEW_SAMPLES],
				NEW_SAMPLES * sizeof(uint32_t));

		osMessageQueuePut(mQueue, &data, 0, 0);
		SensorHandler_NotifyMAX();
	}

	// Should never get h6re
	vTaskSuspend(nullptr);
}
