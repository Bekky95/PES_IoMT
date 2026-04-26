/*
 * pulsOx.h
 *
 *  Created on: 22 Apr 2026
 *      Author: Lucian
 */

#ifndef APP_SPULSOX_MAX3010X_H_
#define APP_SPULSOX_MAX3010X_H_
#include "main.h"
#include "cmsis_os2.h"// TODO implement interrupt source so SensorHandler reacts on interrupt and delviers the source back
enum class InterruptSource {
	PWR_RDY, ALC_OVF
};

#define I2C_BUFFER_LENGTH 32
// Class representing the MAX30102 sensor module
class MAX3010x {
public:
	MAX3010x(I2C_HandleTypeDef *hI2c) :
			mI2c(hI2c) {
	}
	HAL_StatusTypeDef init(void);
	void setup(uint8_t powerLevel = 0x1F, uint8_t sampleAverage = 4,
			uint8_t ledMode = 3, uint16_t sampleRate = 400,
			uint16_t pulseWidth = 411, uint16_t adcRange = 4096);

	uint32_t getRed(void); //Returns immediate red value
	uint32_t getIR(void); //Returns immediate IR value
	uint32_t getGreen(void); //Returns immediate green value
	bool safeCheck(uint8_t maxTimeToCheck); //Given a max amount of time, check for new data

	// Configuration
	void softReset();
	void shutDown();
	void wakeUp();

	void setLEDMode(uint8_t mode);

	void setADCRange(uint8_t adcRange);
	void setSampleRate(uint8_t sampleRate);
	void setPulseWidth(uint8_t pulseWidth);

	void setPulseAmplitudeRed(uint8_t value);
	void setPulseAmplitudeIR(uint8_t value);
	void setPulseAmplitudeGreen(uint8_t value);
	void setPulseAmplitudeProximity(uint8_t value);

	void setProximityThreshold(uint8_t threshMSB);

	//Multi-led configuration mode (page 22)
	void enableSlot(uint8_t slotNumber, uint8_t device); //Given slot number, assign a device to slot
	void disableSlots(void);

	// Data Collection

	//Interrupts (page 13, 14)
	uint8_t getINT1(void); //Returns the main interrupt group
	uint8_t getINT2(void); //Returns the temp ready interrupt
	void enableAFULL(void); //Enable/disable individual interrupts
	void disableAFULL(void);
	void enableDATARDY(void);
	void disableDATARDY(void);
	void enableALCOVF(void);
	void disableALCOVF(void);
	void enablePROXINT(void);
	void disablePROXINT(void);
	void enableDIETEMPRDY(void);
	void disableDIETEMPRDY(void);

	//FIFO Configuration (page 18)
	void setFIFOAverage(uint8_t samples);
	void enableFIFORollover();
	void disableFIFORollover();
	void setFIFOAlmostFull(uint8_t samples);

	//FIFO Reading
	uint16_t check(void); //Checks for new data and fills FIFO
	uint8_t available(void); //Tells caller how many new samples are available (head - tail)
	void nextSample(void); //Advances the tail of the sense array
	uint32_t getFIFORed(void); //Returns the FIFO sample pointed to by tail
	uint32_t getFIFOIR(void); //Returns the FIFO sample pointed to by tail
	uint32_t getFIFOGreen(void); //Returns the FIFO sample pointed to by tail

	uint8_t getWritePointer(void);
	uint8_t getReadPointer(void);
	void clearFIFO(void); //Sets the read/write pointers to zero

	//Proximity Mode Interrupt Threshold
	void setPROXINTTHRESH(uint8_t val);

	// Die Temperature
	float readTemperature();
	float readTemperatureF();

	// Detecting ID/Revision
	uint8_t getRevisionID();
	uint8_t readPartID();

private:
	I2C_HandleTypeDef *mI2c;
	static const uint16_t _i2caddr = 0xAE;

	//activeLEDs is the number of channels turned on, and can be 1 to 3. 2 is common for Red+IR.
	uint8_t activeLEDs; //Gets set during setup. Allows check() to calculate how many bytes to read from FIFO

	uint8_t revisionID;
	void readRevisionID();

#define STORAGE_SIZE 4 //Each long is 4 bytes so limit this to fit on your micro
	typedef struct Record {
		uint32_t red[STORAGE_SIZE];
		uint32_t IR[STORAGE_SIZE];
		uint32_t green[STORAGE_SIZE];
		uint8_t head;
		uint8_t tail;
	} sense_struct; //This is our circular buffer of readings from the sensor

	sense_struct sense;

	// Low-Level read/write functions
	uint8_t readRegister8(uint8_t addr);
	HAL_StatusTypeDef writeRegister8(uint8_t addr, uint8_t data); // TODO: check if ack or the i2c returns something
	void bitMask(uint8_t reg, uint8_t mask, uint8_t thing);

};

#endif /* APP_SPULSOX_MAX3010X_H_ */
