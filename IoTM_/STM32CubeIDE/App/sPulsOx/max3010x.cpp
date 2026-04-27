/*
 * pulsOx.cpp
 *
 *  Created on: 22 Apr 2026
 *      Author: Lucian
 */
#include <sPulsOx/max3010x.h>
// REGISTERS COPIED FROM GITHUB WE HAVE THE MAX30102 IN CASE REG DONT MATCH
// Status Registers
static const uint8_t MAX30105_INTSTAT1 = 0x00;
static const uint8_t MAX30105_INTSTAT2 = 0x01;
static const uint8_t MAX30105_INTENABLE1 = 0x02;
static const uint8_t MAX30105_INTENABLE2 = 0x03;

// FIFO Registers
static const uint8_t MAX30105_FIFOWRITEPTR = 0x04;
static const uint8_t MAX30105_FIFOOVERFLOW = 0x05;
static const uint8_t MAX30105_FIFOREADPTR = 0x06;
static const uint8_t MAX30105_FIFODATA = 0x07;

// Configuration Registers
static const uint8_t MAX30105_FIFOCONFIG = 0x08;
static const uint8_t MAX30105_MODECONFIG = 0x09;
static const uint8_t MAX30105_PARTICLECONFIG = 0x0A; // Note, sometimes listed as "SPO2" config in datasheet (pg. 11)
static const uint8_t MAX30105_LED1_PULSEAMP = 0x0C;
static const uint8_t MAX30105_LED2_PULSEAMP = 0x0D;
static const uint8_t MAX30105_LED3_PULSEAMP = 0x0E;
static const uint8_t MAX30105_LED_PROX_AMP = 0x10;
static const uint8_t MAX30105_MULTILEDCONFIG1 = 0x11;
static const uint8_t MAX30105_MULTILEDCONFIG2 = 0x12;

// Die Temperature Registers
static const uint8_t MAX30105_DIETEMPINT = 0x1F;
static const uint8_t MAX30105_DIETEMPFRAC = 0x20;
static const uint8_t MAX30105_DIETEMPCONFIG = 0x21;

// Proximity Function Registers
static const uint8_t MAX30105_PROXINTTHRESH = 0x30;

// Part ID Registers
static const uint8_t MAX30105_REVISIONID = 0xFE;
static const uint8_t MAX30105_PARTID = 0xFF; // Should always be 0x15. Identical to MAX30102.

// MAX30105 Commands
// Interrupt configuration (pg 13, 14)
static const uint8_t MAX30105_INT_A_FULL_MASK = (uint8_t) ~0b10000000;
static const uint8_t MAX30105_INT_A_FULL_ENABLE = 0x80;
static const uint8_t MAX30105_INT_A_FULL_DISABLE = 0x00;

static const uint8_t MAX30105_INT_DATA_RDY_MASK = (uint8_t) ~0b01000000;
static const uint8_t MAX30105_INT_DATA_RDY_ENABLE = 0x40;
static const uint8_t MAX30105_INT_DATA_RDY_DISABLE = 0x00;

static const uint8_t MAX30105_INT_ALC_OVF_MASK = (uint8_t) ~0b00100000;
static const uint8_t MAX30105_INT_ALC_OVF_ENABLE = 0x20;
static const uint8_t MAX30105_INT_ALC_OVF_DISABLE = 0x00;

static const uint8_t MAX30105_INT_PROX_INT_MASK = (uint8_t) ~0b00010000;
static const uint8_t MAX30105_INT_PROX_INT_ENABLE = 0x10;
static const uint8_t MAX30105_INT_PROX_INT_DISABLE = 0x00;

static const uint8_t MAX30105_INT_DIE_TEMP_RDY_MASK = (uint8_t) ~0b00000010;
static const uint8_t MAX30105_INT_DIE_TEMP_RDY_ENABLE = 0x02;
static const uint8_t MAX30105_INT_DIE_TEMP_RDY_DISABLE = 0x00;

static const uint8_t MAX30105_SAMPLEAVG_MASK = (uint8_t) ~0b11100000;
static const uint8_t MAX30105_SAMPLEAVG_1 = 0x00;
static const uint8_t MAX30105_SAMPLEAVG_2 = 0x20;
static const uint8_t MAX30105_SAMPLEAVG_4 = 0x40;
static const uint8_t MAX30105_SAMPLEAVG_8 = 0x60;
static const uint8_t MAX30105_SAMPLEAVG_16 = 0x80;
static const uint8_t MAX30105_SAMPLEAVG_32 = 0xA0;

static const uint8_t MAX30105_ROLLOVER_MASK = 0xEF;
static const uint8_t MAX30105_ROLLOVER_ENABLE = 0x10;
static const uint8_t MAX30105_ROLLOVER_DISABLE = 0x00;

static const uint8_t MAX30105_A_FULL_MASK = 0xF0;

// Mode configuration commands (page 19)
static const uint8_t MAX30105_SHUTDOWN_MASK = 0x7F;
static const uint8_t MAX30105_SHUTDOWN = 0x80;
static const uint8_t MAX30105_WAKEUP = 0x00;

static const uint8_t MAX30105_RESET_MASK = 0xBF;
static const uint8_t MAX30105_RESET = 0x40;

static const uint8_t MAX30105_MODE_MASK = 0xF8;
static const uint8_t MAX30105_MODE_REDONLY = 0x02;
static const uint8_t MAX30105_MODE_REDIRONLY = 0x03;
static const uint8_t MAX30105_MODE_MULTILED = 0x07;

// Particle sensing configuration commands (pgs 19-20)
static const uint8_t MAX30105_ADCRANGE_MASK = 0x9F;
static const uint8_t MAX30105_ADCRANGE_2048 = 0x00;
static const uint8_t MAX30105_ADCRANGE_4096 = 0x20;
static const uint8_t MAX30105_ADCRANGE_8192 = 0x40;
static const uint8_t MAX30105_ADCRANGE_16384 = 0x60;

static const uint8_t MAX30105_SAMPLERATE_MASK = 0xE3;
static const uint8_t MAX30105_SAMPLERATE_50 = 0x00;
static const uint8_t MAX30105_SAMPLERATE_100 = 0x04;
static const uint8_t MAX30105_SAMPLERATE_200 = 0x08;
static const uint8_t MAX30105_SAMPLERATE_400 = 0x0C;
static const uint8_t MAX30105_SAMPLERATE_800 = 0x10;
static const uint8_t MAX30105_SAMPLERATE_1000 = 0x14;
static const uint8_t MAX30105_SAMPLERATE_1600 = 0x18;
static const uint8_t MAX30105_SAMPLERATE_3200 = 0x1C;

static const uint8_t MAX30105_PULSEWIDTH_MASK = 0xFC;
static const uint8_t MAX30105_PULSEWIDTH_69 = 0x00;
static const uint8_t MAX30105_PULSEWIDTH_118 = 0x01;
static const uint8_t MAX30105_PULSEWIDTH_215 = 0x02;
static const uint8_t MAX30105_PULSEWIDTH_411 = 0x03;

//Multi-LED Mode configuration (pg 22)
static const uint8_t MAX30105_SLOT1_MASK = 0xF8;
static const uint8_t MAX30105_SLOT2_MASK = 0x8F;
static const uint8_t MAX30105_SLOT3_MASK = 0xF8;
static const uint8_t MAX30105_SLOT4_MASK = 0x8F;

static const uint8_t SLOT_NONE = 0x00;
static const uint8_t SLOT_RED_LED = 0x01;
static const uint8_t SLOT_IR_LED = 0x02;
static const uint8_t SLOT_GREEN_LED = 0x03;
static const uint8_t SLOT_NONE_PILOT = 0x04;
static const uint8_t SLOT_RED_PILOT = 0x05;
static const uint8_t SLOT_IR_PILOT = 0x06;
static const uint8_t SLOT_GREEN_PILOT = 0x07;

static const uint8_t MAX_30105_EXPECTEDPARTID = 0x15;

//Begin Interrupt configuration
uint8_t MAX3010x::getINT1(void) {
	return (readRegister8(MAX30105_INTSTAT1));
}
uint8_t MAX3010x::getINT2(void) {
	return (readRegister8(MAX30105_INTSTAT2));
}

void MAX3010x::enableAFULL(void) {
	bitMask(MAX30105_INTENABLE1, MAX30105_INT_A_FULL_MASK,
			MAX30105_INT_A_FULL_ENABLE);
}
void MAX3010x::disableAFULL(void) {
	bitMask(MAX30105_INTENABLE1, MAX30105_INT_A_FULL_MASK,
			MAX30105_INT_A_FULL_DISABLE);
}

void MAX3010x::enableDATARDY(void) {
	bitMask(MAX30105_INTENABLE1, MAX30105_INT_DATA_RDY_MASK,
			MAX30105_INT_DATA_RDY_ENABLE);
}
void MAX3010x::disableDATARDY(void) {
	bitMask(MAX30105_INTENABLE1, MAX30105_INT_DATA_RDY_MASK,
			MAX30105_INT_DATA_RDY_DISABLE);
}

void MAX3010x::enableALCOVF(void) {
	bitMask(MAX30105_INTENABLE1, MAX30105_INT_ALC_OVF_MASK,
			MAX30105_INT_ALC_OVF_ENABLE);
}
void MAX3010x::disableALCOVF(void) {
	bitMask(MAX30105_INTENABLE1, MAX30105_INT_ALC_OVF_MASK,
			MAX30105_INT_ALC_OVF_DISABLE);
}

void MAX3010x::enablePROXINT(void) {
	bitMask(MAX30105_INTENABLE1, MAX30105_INT_PROX_INT_MASK,
			MAX30105_INT_PROX_INT_ENABLE);
}
void MAX3010x::disablePROXINT(void) {
	bitMask(MAX30105_INTENABLE1, MAX30105_INT_PROX_INT_MASK,
			MAX30105_INT_PROX_INT_DISABLE);
}

void MAX3010x::enableDIETEMPRDY(void) {
	bitMask(MAX30105_INTENABLE2, MAX30105_INT_DIE_TEMP_RDY_MASK,
			MAX30105_INT_DIE_TEMP_RDY_ENABLE);
}
void MAX3010x::disableDIETEMPRDY(void) {
	bitMask(MAX30105_INTENABLE2, MAX30105_INT_DIE_TEMP_RDY_MASK,
			MAX30105_INT_DIE_TEMP_RDY_DISABLE);
}

//End Interrupt configuration

void MAX3010x::softReset(void) {
	bitMask(MAX30105_MODECONFIG, MAX30105_RESET_MASK, MAX30105_RESET);

	// Poll for bit to clear, reset is then complete
	// Timeout after 100ms
	uint32_t startTime = HAL_GetTick();
	while (HAL_GetTick() - startTime < 100) {
		uint8_t response = readRegister8(MAX30105_MODECONFIG);
		if ((response & MAX30105_RESET) == 0)
			break; // We're done!
		HAL_Delay(1); // Let's not over burden the I2C bus
	}
}

void MAX3010x::shutDown(void) {
	// Put IC into low power mode (datasheet pg. 19)
	// During shutdown the IC will continue to respond to I2C commands but will
	// not update with or take new readings (such as temperature)
	bitMask(MAX30105_MODECONFIG, MAX30105_SHUTDOWN_MASK, MAX30105_SHUTDOWN);
}

void MAX3010x::wakeUp(void) {
	// Pull IC out of low power mode (datasheet pg. 19)
	bitMask(MAX30105_MODECONFIG, MAX30105_SHUTDOWN_MASK, MAX30105_WAKEUP);
}

void MAX3010x::setLEDMode(uint8_t mode) {
	// Set which LEDs are used for sampling -- Red only, RED+IR only, or custom.
	// See datasheet, page 19
	bitMask(MAX30105_MODECONFIG, MAX30105_MODE_MASK, mode);
}

void MAX3010x::setADCRange(uint8_t adcRange) {
	// adcRange: one of MAX30105_ADCRANGE_2048, _4096, _8192, _16384
	bitMask(MAX30105_PARTICLECONFIG, MAX30105_ADCRANGE_MASK, adcRange);
}

void MAX3010x::setSampleRate(uint8_t sampleRate) {
	// sampleRate: one of MAX30105_SAMPLERATE_50, _100, _200, _400, _800, _1000, _1600, _3200
	bitMask(MAX30105_PARTICLECONFIG, MAX30105_SAMPLERATE_MASK, sampleRate);
}

void MAX3010x::setPulseWidth(uint8_t pulseWidth) {
	// pulseWidth: one of MAX30105_PULSEWIDTH_69, _188, _215, _411
	bitMask(MAX30105_PARTICLECONFIG, MAX30105_PULSEWIDTH_MASK, pulseWidth);
}

// NOTE: Amplitude values: 0x00 = 0mA, 0x7F = 25.4mA, 0xFF = 50mA (typical)
// See datasheet, page 21
void MAX3010x::setPulseAmplitudeRed(uint8_t amplitude) {
	writeRegister8(MAX30105_LED1_PULSEAMP, amplitude);
}

void MAX3010x::setPulseAmplitudeIR(uint8_t amplitude) {
	writeRegister8(MAX30105_LED2_PULSEAMP, amplitude);
}

void MAX3010x::setPulseAmplitudeGreen(uint8_t amplitude) {
	writeRegister8(MAX30105_LED3_PULSEAMP, amplitude);
}

void MAX3010x::setPulseAmplitudeProximity(uint8_t amplitude) {
	writeRegister8(MAX30105_LED_PROX_AMP, amplitude);
}

void MAX3010x::setProximityThreshold(uint8_t threshMSB) {
	// Set the IR ADC count that will trigger the beginning of particle-sensing mode.
	// The threshMSB signifies only the 8 most significant-bits of the ADC count.
	// See datasheet, page 24.
	writeRegister8(MAX30105_PROXINTTHRESH, threshMSB);
}

//Given a slot number assign a thing to it
//Devices are SLOT_RED_LED or SLOT_RED_PILOT (proximity)
//Assigning a SLOT_RED_LED will pulse LED
//Assigning a SLOT_RED_PILOT will ??
void MAX3010x::enableSlot(uint8_t slotNumber, uint8_t device) {

	uint8_t originalContents;

	switch (slotNumber) {
	case (1):
		bitMask(MAX30105_MULTILEDCONFIG1, MAX30105_SLOT1_MASK, device);
		break;
	case (2):
		bitMask(MAX30105_MULTILEDCONFIG1, MAX30105_SLOT2_MASK, device << 4);
		break;
	case (3):
		bitMask(MAX30105_MULTILEDCONFIG2, MAX30105_SLOT3_MASK, device);
		break;
	case (4):
		bitMask(MAX30105_MULTILEDCONFIG2, MAX30105_SLOT4_MASK, device << 4);
		break;
	default:
		//Shouldn't be here!
		break;
	}
}

//Clears all slot assignments
void MAX3010x::disableSlots(void) {
	writeRegister8(MAX30105_MULTILEDCONFIG1, 0);
	writeRegister8(MAX30105_MULTILEDCONFIG2, 0);
}

//
// FIFO Configuration
//

//Set sample average (Table 3, Page 18)
void MAX3010x::setFIFOAverage(uint8_t numberOfSamples) {
	bitMask(MAX30105_FIFOCONFIG, MAX30105_SAMPLEAVG_MASK, numberOfSamples);
}

//Resets all points to start in a known state
//Page 15 recommends clearing FIFO before beginning a read
void MAX3010x::clearFIFO(void) {
	writeRegister8(MAX30105_FIFOWRITEPTR, 0);
	writeRegister8(MAX30105_FIFOOVERFLOW, 0);
	writeRegister8(MAX30105_FIFOREADPTR, 0);
}

//Enable roll over if FIFO over flows
void MAX3010x::enableFIFORollover(void) {
	bitMask(MAX30105_FIFOCONFIG, MAX30105_ROLLOVER_MASK,
			MAX30105_ROLLOVER_ENABLE);
}

//Disable roll over if FIFO over flows
void MAX3010x::disableFIFORollover(void) {
	bitMask(MAX30105_FIFOCONFIG, MAX30105_ROLLOVER_MASK,
			MAX30105_ROLLOVER_DISABLE);
}

//Set number of samples to trigger the almost full interrupt (Page 18)
//Power on default is 32 samples
//Note it is reverse: 0x00 is 32 samples, 0x0F is 17 samples
void MAX3010x::setFIFOAlmostFull(uint8_t numberOfSamples) {
	bitMask(MAX30105_FIFOCONFIG, MAX30105_A_FULL_MASK, numberOfSamples);
}

//Read the FIFO Write Pointer
uint8_t MAX3010x::getWritePointer(void) {
	return (readRegister8(MAX30105_FIFOWRITEPTR));
}

//Read the FIFO Read Pointer
uint8_t MAX3010x::getReadPointer(void) {
	return (readRegister8(MAX30105_FIFOREADPTR));
}

// Die Temperature
// Returns temp in C
float MAX3010x::readTemperature() {

	//DIE_TEMP_RDY interrupt must be enabled
	//See issue 19: https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library/issues/19

	// Step 1: Config die temperature register to take 1 temperature sample
	writeRegister8(MAX30105_DIETEMPCONFIG, 0x01);

	// Poll for bit to clear, reading is then complete
	// Timeout after 100ms
	uint32_t startTime = HAL_GetTick();
	while (HAL_GetTick() - startTime < 100) {
		//uint8_t response = readRegister8(_i2caddr, MAX30105_DIETEMPCONFIG); //Original way
		//if ((response & 0x01) == 0) break; //We're done!

		//Check to see if DIE_TEMP_RDY interrupt is set
		uint8_t response = readRegister8(MAX30105_INTSTAT2);
		if ((response & MAX30105_INT_DIE_TEMP_RDY_ENABLE) > 0)
			break; //We're done!
		osDelay(1); //Let's not over burden the I2C bus
	}
	//TODO How do we want to fail? With what type of error?
	//? if(millis() - startTime >= 100) return(-999.0);

	// Step 2: Read die temperature register (integer)
	int8_t tempInt = readRegister8(MAX30105_DIETEMPINT);
	uint8_t tempFrac = readRegister8(MAX30105_DIETEMPFRAC); //Causes the clearing of the DIE_TEMP_RDY interrupt

	// Step 3: Calculate temperature (datasheet pg. 23)
	return (float) tempInt + ((float) tempFrac * 0.0625);
}

// Returns die temp in F
float MAX3010x::readTemperatureF() {
	float temp = readTemperature();

	if (temp != -999.0)
		temp = temp * 1.8 + 32.0;

	return (temp);
}

// Set the PROX_INT_THRESHold
void MAX3010x::setPROXINTTHRESH(uint8_t val) {
	writeRegister8(MAX30105_PROXINTTHRESH, val);
}

//
// Device ID and Revision
//
uint8_t MAX3010x::readPartID() {
	return readRegister8(MAX30105_PARTID);
}

void MAX3010x::readRevisionID() {
	revisionID = readRegister8(MAX30105_REVISIONID);
}

uint8_t MAX3010x::getRevisionID() {
	return revisionID;
}

//Setup the sensorsb
//The MAX30105 has many settings. By default we select:
// Sample Average = 4
// Mode = MultiLED
// ADC Range = 16384 (62.5pA per LSB)
// Sample rate = 50
//Use the default setup if you are just getting started with the MAX30105 sensor
void MAX3010x::setup(uint8_t powerLevel, uint8_t sampleAverage, uint8_t ledMode,
		uint16_t sampleRate, uint16_t pulseWidth, uint16_t adcRange) {
	softReset(); //Reset all configuration, threshold, and data registers to POR values

	//FIFO Configuration
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	//The chip will average multiple samples of same type together if you wish
	if (sampleAverage == 1)
		setFIFOAverage(MAX30105_SAMPLEAVG_1); //No averaging per FIFO record
	else if (sampleAverage == 2)
		setFIFOAverage(MAX30105_SAMPLEAVG_2);
	else if (sampleAverage == 4)
		setFIFOAverage(MAX30105_SAMPLEAVG_4);
	else if (sampleAverage == 8)
		setFIFOAverage(MAX30105_SAMPLEAVG_8);
	else if (sampleAverage == 16)
		setFIFOAverage(MAX30105_SAMPLEAVG_16);
	else if (sampleAverage == 32)
		setFIFOAverage(MAX30105_SAMPLEAVG_32);
	else
		setFIFOAverage(MAX30105_SAMPLEAVG_4);

	//setFIFOAlmostFull(2); //Set to 30 samples to trigger an 'Almost Full' interrupt
	enableFIFORollover(); //Allow FIFO to wrap/roll over
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	//Mode Configuration
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (ledMode == 3)
		setLEDMode(MAX30105_MODE_MULTILED); //Watch all three LED channels
	else if (ledMode == 2)
		setLEDMode(MAX30105_MODE_REDIRONLY); //Red and IR
	else
		setLEDMode(MAX30105_MODE_REDONLY); //Red only
	activeLEDs = ledMode; //Used to control how many bytes to read from FIFO buffer
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	//Particle Sensing Configuration
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (adcRange < 4096)
		setADCRange(MAX30105_ADCRANGE_2048); //7.81pA per LSB
	else if (adcRange < 8192)
		setADCRange(MAX30105_ADCRANGE_4096); //15.63pA per LSB
	else if (adcRange < 16384)
		setADCRange(MAX30105_ADCRANGE_8192); //31.25pA per LSB
	else if (adcRange == 16384)
		setADCRange(MAX30105_ADCRANGE_16384); //62.5pA per LSB
	else
		setADCRange(MAX30105_ADCRANGE_2048);

	if (sampleRate < 100)
		setSampleRate(MAX30105_SAMPLERATE_50); //Take 50 samples per second
	else if (sampleRate < 200)
		setSampleRate(MAX30105_SAMPLERATE_100);
	else if (sampleRate < 400)
		setSampleRate(MAX30105_SAMPLERATE_200);
	else if (sampleRate < 800)
		setSampleRate(MAX30105_SAMPLERATE_400);
	else if (sampleRate < 1000)
		setSampleRate(MAX30105_SAMPLERATE_800);
	else if (sampleRate < 1600)
		setSampleRate(MAX30105_SAMPLERATE_1000);
	else if (sampleRate < 3200)
		setSampleRate(MAX30105_SAMPLERATE_1600);
	else if (sampleRate == 3200)
		setSampleRate(MAX30105_SAMPLERATE_3200);
	else
		setSampleRate(MAX30105_SAMPLERATE_50);

	//The longer the pulse width the longer range of detection you'll have
	//At 69us and 0.4mA it's about 2 inches
	//At 411us and 0.4mA it's about 6 inches
	if (pulseWidth < 118)
		setPulseWidth(MAX30105_PULSEWIDTH_69); //Page 26, Gets us 15 bit resolution
	else if (pulseWidth < 215)
		setPulseWidth(MAX30105_PULSEWIDTH_118); //16 bit resolution
	else if (pulseWidth < 411)
		setPulseWidth(MAX30105_PULSEWIDTH_215); //17 bit resolution
	else if (pulseWidth == 411)
		setPulseWidth(MAX30105_PULSEWIDTH_411); //18 bit resolution
	else
		setPulseWidth(MAX30105_PULSEWIDTH_69);
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	//LED Pulse Amplitude Configuration
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	//Default is 0x1F which gets us 6.4mA
	//powerLevel = 0x02, 0.4mA - Presence detection of ~4 inch
	//powerLevel = 0x1F, 6.4mA - Presence detection of ~8 inch
	//powerLevel = 0x7F, 25.4mA - Presence detection of ~8 inch
	//powerLevel = 0xFF, 50.0mA - Presence detection of ~12 inch

	setPulseAmplitudeRed(powerLevel);
	setPulseAmplitudeIR(powerLevel);
	setPulseAmplitudeGreen(powerLevel);
	setPulseAmplitudeProximity(powerLevel);
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	//Multi-LED Mode Configuration, Enable the reading of the three LEDs
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	enableSlot(1, SLOT_RED_LED);
	if (ledMode > 1)
		enableSlot(2, SLOT_IR_LED);
	if (ledMode > 2)
		enableSlot(3, SLOT_GREEN_LED);
	//enableSlot(1, SLOT_RED_PILOT);
	//enableSlot(2, SLOT_IR_PILOT);
	//enableSlot(3, SLOT_GREEN_PILOT);
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	clearFIFO(); //Reset the FIFO before we begin checking the sensor
}

//
// Data Collection
//

//Tell caller how many samples are available
uint8_t MAX3010x::available(void) {
	int8_t numberOfSamples = sense.head - sense.tail;
	if (numberOfSamples < 0)
		numberOfSamples += STORAGE_SIZE;

	return (numberOfSamples);
}

//Report the most recent red value
uint32_t MAX3010x::getRed(void) {
	//Check the sensor for new data for 250ms
	if (safeCheck(250))
		return (sense.red[sense.head]);
	else
		return (0); //Sensor failed to find new data
}

//Report the most recent IR value
uint32_t MAX3010x::getIR(void) {
	//Check the sensor for new data for 250ms
	if (safeCheck(250))
		return (sense.IR[sense.head]);
	else
		return (0); //Sensor failed to find new data
}

//Report the most recent Green value
uint32_t MAX3010x::getGreen(void) {
	//Check the sensor for new data for 250ms
	if (safeCheck(250))
		return (sense.green[sense.head]);
	else
		return (0); //Sensor failed to find new data
}

//Report the next Red value in the FIFO
uint32_t MAX3010x::getFIFORed(void) {
	return (sense.red[sense.tail]);
}

//Report the next IR value in the FIFO
uint32_t MAX3010x::getFIFOIR(void) {
	return (sense.IR[sense.tail]);
}

//Report the next Green value in the FIFO
uint32_t MAX3010x::getFIFOGreen(void) {
	return (sense.green[sense.tail]);
}

//Advance the tail
void MAX3010x::nextSample(void) {
	if (available()) //Only advance the tail if new data is available
	{
		sense.tail++;
		sense.tail %= STORAGE_SIZE; //Wrap condition
	}
}

//Polls the sensor for new data
//Call regularly
//If new data is available, it updates the head and tail in the main struct
//Returns number of new samples obtained
uint16_t MAX3010x::check(void) {
	uint8_t readPointer = getReadPointer();
	uint8_t writePointer = getWritePointer();

	int numberOfSamples = 0;

	if (readPointer != writePointer) {
		numberOfSamples = writePointer - readPointer;
		if (numberOfSamples < 0)
			numberOfSamples += 32;

		int bytesLeftToRead = numberOfSamples * activeLEDs * 3;

		// Point the sensor's internal pointer at the FIFO data register
		uint8_t reg = MAX30105_FIFODATA;
		HAL_I2C_Master_Transmit(mI2c, _i2caddr, &reg, 1, HAL_MAX_DELAY);

		while (bytesLeftToRead > 0) {
			int toGet = bytesLeftToRead;

			// Trim to a multiple of bytes-per-sample so we never split a sample
			if (toGet > I2C_BUFFER_LENGTH)
				toGet = I2C_BUFFER_LENGTH
						- (I2C_BUFFER_LENGTH % (activeLEDs * 3));

			bytesLeftToRead -= toGet;

			// Read toGet bytes in one burst
			uint8_t buf[toGet];
			HAL_I2C_Master_Receive(mI2c, _i2caddr, buf, toGet,
			HAL_MAX_DELAY);

			uint8_t *p = buf;

			while (toGet > 0) {
				sense.head = (sense.head + 1) % STORAGE_SIZE;

				uint32_t tempLong;

				// --- RED (3 bytes, MSB first) ---
				tempLong = ((uint32_t) p[0] << 16) | ((uint32_t) p[1] << 8)
						| (uint32_t) p[2];
				p += 3;
				tempLong &= 0x3FFFF;
				sense.red[sense.head] = tempLong;

				// --- IR ---
				if (activeLEDs > 1) {
					tempLong = ((uint32_t) p[0] << 16) | ((uint32_t) p[1] << 8)
							| (uint32_t) p[2];
					p += 3;
					tempLong &= 0x3FFFF;
					sense.IR[sense.head] = tempLong;
				}

				// --- Green ---
				if (activeLEDs > 2) {
					tempLong = ((uint32_t) p[0] << 16) | ((uint32_t) p[1] << 8)
							| (uint32_t) p[2];
					p += 3;
					tempLong &= 0x3FFFF;
					sense.green[sense.head] = tempLong;
				}

				toGet -= activeLEDs * 3;
			}
		}
	}

	return (uint16_t) numberOfSamples;
}

//Check for new data but give up after a certain amount of time
//Returns true if new data was found
//Returns false if new data was not found
bool MAX3010x::safeCheck(TickType_t  maxTimeToCheck) {
	uint32_t markTime = HAL_GetTick();

	while (1) {
		if (HAL_GetTick() - markTime > maxTimeToCheck)
			return (false);

		if (check() == true) //We found new data!
			return (true);

		osDelay(1);
	}
}

uint8_t MAX3010x::readRegister8(uint8_t addr) {
	uint8_t data = 0;
	if (HAL_I2C_Mem_Read(mI2c, _i2caddr, addr, I2C_MEMADD_SIZE_8BIT, &data, 1,
	HAL_MAX_DELAY) != HAL_OK) {
		return 0;
	}
	return data;
}

HAL_StatusTypeDef MAX3010x::writeRegister8(uint8_t addr, uint8_t data) {
	return HAL_I2C_Mem_Write(mI2c, _i2caddr, addr, I2C_MEMADD_SIZE_8BIT, &data,
			1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MAX3010x::init(void) {
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t partID = readRegister8(MAX30105_PARTID);
	if (MAX_30105_EXPECTEDPARTID != partID) {
		status = HAL_ERROR;
	}
	return status;
}

//Given a register, read it, mask it, and then set the thing
void MAX3010x::bitMask(uint8_t reg, uint8_t mask, uint8_t thing) {
	// Grab current register context
	uint8_t originalContents = readRegister8(reg);

	// Zero-out the portions of the register we're interested in
	originalContents = originalContents & mask;

	// Change contents
	writeRegister8(reg, originalContents | thing);
}

