#pragma once
#include "FreeRTOS.h"
#include "main.h"
#include <stdint.h>
#include "queue.h"
#include "semphr.h"
#include <stdbool.h>
#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

// Debug defines, determine which senor is currently used
#define USE_EEG_SENSOR	true
#define USE_EMG_SENSOR  true
#define USE_EKG_SENSOR  true
#define USE_ADC_SENSORS	(USE_EEG_SENSOR | USE_EMG_SENSOR |USE_EKG_SENSOR)
#define USE_SP02_SENSOR false

#define USE_UI 			true
#define USE_MQTT_CONN   false

//
#define ADC_CH_COUNT (USE_EEG_SENSOR + USE_EMG_SENSOR + USE_EKG_SENSOR)

typedef enum {
	MAX1030x, EMG, EEG, EKG
} SensorType;

typedef struct {
	int32_t spo2;
	int8_t validSPO2;
	int32_t heartRate;
	int8_t validHeartRate;
} MAX3010x_Data;

//TODO if needed change this to hold data type + pointer to data to save space
typedef struct {
	SensorType type;
	uint32_t timestamp_ms;

	union {
		uint16_t EmgData;
		uint16_t EegData;
		uint16_t EkgData;
		MAX3010x_Data SpO2Data;
	};
} SensorData;

typedef struct {
	ADC_HandleTypeDef *hadc;
	uint8_t adcChannelCount;
	I2C_HandleTypeDef *hi2c;
	QueueHandle_t uiQueue;
	osMessageQueueId_t adcQueue;
	osMessageQueueId_t max3010xQueue;
	SemaphoreHandle_t uiSem;
} SensorHandlerConfig;

typedef struct {
	osMessageQueueId_t queue;
	I2C_HandleTypeDef *hi2c;
} SpO2Config;

typedef struct {
	osMessageQueueId_t queue;
	ADC_HandleTypeDef *adc;
	uint8_t adcChannelCount;
} adcConfig;

typedef struct {
    uint16_t values[ADC_CH_COUNT];
    uint32_t timestamp_ms;
} AdcSnapshot;

typedef enum {
    ADC_CH_EMG  = 0,
    ADC_CH_EEG  = 1,
    ADC_CH_EKG  = 2,

} AdcChannel;

// Sensor_Handler_Notifybits
#define SENSOR_HANDLER_NOTIFYBITS_NEW_ADC_DATA  (1UL << 0)
#define SENSOR_HANDLER_NOTIFYBITS_NEW_MAX_DATA  (1UL << 1)
/* TODO
 * #define SENSOR_HANDLER_NOTIFYBITS_NEW_MQTT_PACKET  (1UL << 2)
 * #define SENSOR_HANDLER_NOTIFYBITS_ADC_ERROR        (1UL << 3)
 * usw...
 */
#ifdef __cplusplus
}
#endif
