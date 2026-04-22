/*
 * pulsOx.h
 *
 *  Created on: 22 Apr 2026
 *      Author: Lucian
 */

#ifndef APP_SPULSOX_PULSOX_H_
#define APP_SPULSOX_PULSOX_H_
#include "main.c"
// TODO implement interrupt source so SensorHandler reacts on interrupt and delviers the source back
enum class InterruptSource  {
	PWR_RDY, ALC_OVF
};
// Class representing the MAX30102 sensor module
class PulsOx {
public:
	PulsOx(I2C_HandleTypeDef* hI2c);
	//TODO: maybe private these and hide them behind functions such as enable_X, disable_Y, get_Z usw...
	uint8_t readReg(uint8_t addr);
	void writeReg(uint8_t addr, uint8_t data); // TODO: check if ack or the i2c returns something

private:
	I2C_HandleTypeDef* mI2c;
	static const uint8_t writeAddr = 0xAE;
	static const uint8_t readAddr = 0xAF;
};


#endif /* APP_SPULSOX_PULSOX_H_ */
