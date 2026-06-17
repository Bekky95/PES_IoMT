#ifndef SENSOR_PACKET_H
#define SENSOR_PACKET_H

#include <stdint.h>
#include <stddef.h>

// Protocol definitions:
#define UART_SYNC1 0xAA
#define UART_SYNC2 0x55

#define MAX_BATCH_SIZE  8
#define MAX_PAYLOAD_SIZE  80  

typedef enum {
    SENSOR_NONE = 0,
    ADC_COMBINED = 1,
    EMG = 2,
    EEG = 3,
    EKG = 4,
    MAX1030x = 5,
    MAX_Sp02 = 6,
    MAX_HR = 7
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