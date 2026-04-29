#pragma once

#include "main.h"
#include "queue.h"
#include "semphr.h"
#include <stdbool.h>
#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	MAX1030x,
	EMG,
	EEG,
	EKG
} SensorType;

//TODO if needed change this to hold data type + pointer to data to save space
typedef struct  {
    SensorType type;
    uint32_t timestamp_ms;

    union {
    	uint16_t	EmgData;
    	uint16_t	EegData;
    	uint16_t	EkgData;
    	uint16_t	SpO2Data;
    	uint16_t	HeartRateData;
    };
}SensorData;


typedef struct  {
	ADC_HandleTypeDef* 	hadc;
	uint8_t 			adcChannelCount;
	I2C_HandleTypeDef*	hi2c;
	uint16_t			i2cAddress; //7-bit, should be left shifted by HAL
	uint16_t			i2cReadBytes;
	uint32_t			loopPeriodMs;
	QueueHandle_t   	uiQueue;
	SemaphoreHandle_t     uiSem;
} SensorHandlerConfig;

typedef struct {
    osMessageQueueId_t  queue;
    I2C_HandleTypeDef*  hi2c;
}SpO2Config;

typedef struct {
	osMessageQueueId_t queue;
	ADC_HandleTypeDef* adc;
	uint8_t		adcChannelCount;
}adcConfig;

#ifdef __cplusplus
}
#endif
