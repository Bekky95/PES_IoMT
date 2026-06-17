#ifndef SENSOR_PACKET_H
#define SENSOR_PACKET_H

#include <stdint.h>
#include <stddef.h>

// Protocol definitions:
#define UART_SYNC1 0xAA
#define UART_SYNC2 0x55

#define MAX_BATCH_SIZE  8
#define MAX_PAYLOAD_SIZE  80  

typedef struct {
	int32_t spo2;
	int8_t validSPO2;
	int32_t heartRate;
	int8_t validHeartRate;
} MAX3010x_Data;

typedef enum {
	MAX1030x, ADC_COMBINED, EMG, EEG, EKG, MAX_Sp02, MAX_HR, SENSOR_NONE
} SensorType;

typedef struct __attribute__((packed))
{
    uint8_t type;
    uint8_t count;
    uint32_t timestamp;
} PacketHeader;

// Packet Container
typedef struct
{
    PacketHeader header;
    uint8_t payload[80]; // 80 being the max payload size
} SensorPacket;

#endif // SENSOR_PACKET_H