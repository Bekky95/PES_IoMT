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
#define USE_EKG_SENSOR  true		//PIN: PB-1
#define USE_ADC_SENSORS	(USE_EEG_SENSOR | USE_EMG_SENSOR |USE_EKG_SENSOR)
#define USE_SP02_SENSOR false

#define USE_UI 			true
#define USE_MQTT   		false

//
#define ADC_CH_COUNT (USE_EEG_SENSOR + USE_EMG_SENSOR + USE_EKG_SENSOR)
#define ADC_BLOCK_SIZE	10


// ADC data reader helper
#define ADC_EMG(buf, i) ((buf)[(i) * ADC_CH_COUNT + 0])
#define ADC_EKG(buf, i) ((buf)[(i) * ADC_CH_COUNT + 1])
#define ADC_EEG(buf, i) ((buf)[(i) * ADC_CH_COUNT + 2])

typedef enum {
	MAX1030x, ADC_COMBINED, EMG, EEG, EKG, MAX_Sp02, MAX_HR, SENSOR_NONE
} SensorType;

// helper function to show that MAX1030x holds both sensor types
static inline int is_max_sensor(SensorType t) {
	return t == MAX1030x || t == MAX_Sp02 || t == MAX_HR;
}
static inline int is_adc_sensor(SensorType t) {
	return t == EMG || t == EKG || t == EKG || t == ADC_COMBINED;
}

typedef struct {
	int32_t spo2;
	int8_t validSPO2;
	int32_t heartRate;
	int8_t validHeartRate;
} MAX3010x_Data;

typedef struct {
	uint16_t emg;
	uint16_t ekg;
	uint16_t eeg;
} AdcSensorData;

typedef struct {
	uint16_t values[ADC_CH_COUNT * ADC_BLOCK_SIZE];
	uint8_t  count;
} AdcSnapshot;
//TODO if needed change this to hold data type + pointer to data to save space
typedef struct {
	SensorType type;
	uint32_t timestamp_ms;

	union {
		AdcSnapshot AdcData;
		MAX3010x_Data SpO2Data;
	};
} SensorData;

typedef struct {
	ADC_HandleTypeDef *hadc;
	uint8_t adcChannelCount;
	I2C_HandleTypeDef *hi2c;
	QueueHandle_t uiQueue;
	QueueHandle_t adcQueue;
	osMessageQueueId_t max3010xQueue;
	osMessageQueueId_t uartQueue;
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
	osMessageQueueId_t queue;
	UART_HandleTypeDef *uart;
} uartConfig;


typedef enum {
	ADC_CH_EMG = 0, ADC_CH_EEG = 1, ADC_CH_EKG = 2,
} AdcChannel;

// Sensor_Handler_Notifybits
#define SENSOR_HANDLER_NOTIFYBITS_NEW_ADC_DATA  (1UL << 0)
#define SENSOR_HANDLER_NOTIFYBITS_NEW_MAX_DATA  (1UL << 1)
/* TODO
 * #define SENSOR_HANDLER_NOTIFYBITS_NEW_MQTT_PACKET  (1UL << 2)
 * #define SENSOR_HANDLER_NOTIFYBITS_ADC_ERROR        (1UL << 3)
 * usw...
 */

// UART task notify bits
#define UART_HANDLER_NEW_TX_DATA (1UL << 0)
#ifdef __cplusplus
}
#endif
