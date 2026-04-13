#pragma once

#include "main.h"
#include "queue.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
	uint16_t adc[3]; // one value per ADC Sensor
	uint16_t pulsOxValue;			// PulsOx value read per i2c
	bool	 pulsOxValid;			// false if last i2c read failed
}SensorData;

typedef struct  {
	ADC_HandleTypeDef* 	hadc;
	uint8_t 			adcChannelCount;
	I2C_HandleTypeDef*	hi2c;
	uint16_t			i2cAddress; //7-bit, should be left shifted by HAL
	uint16_t			i2cReadBytes;
	uint32_t			loopPeriodMs;
	QueueHandle_t   	uiQueue;
} SensorHandlerConfig;

#ifdef __cplusplus
}
#endif
