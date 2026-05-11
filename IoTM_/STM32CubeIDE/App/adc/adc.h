/*
 * adc.hpp
 *
 *  Created on: 12 Apr 2026
 *      Author: Lucian
 */

#ifndef APP_ADC_H_
#define APP_ADC_H_

// Includes
#include <stdint.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "Config.h"
#include <cstring>

static const float VREF = 3.3f;

class AdcDma {
public:

	AdcDma(ADC_HandleTypeDef* hadc, uint8_t numChannels);
    // Constructor — no hardware access, no assertions
    AdcDma() : mHadc(nullptr), mNumChannels(0)
    {
        memset(mDmaBuffer,    0, sizeof(mDmaBuffer));
    }
    //TODO the mHadc seems to be getting a copy and not the actual register pointer FIX!!
	HAL_StatusTypeDef start();
	HAL_StatusTypeDef stop();

	uint16_t* getValues();
	float getChannelValue(uint8_t ch) const;
	float GetChValVolt(uint8_t ch) const;


private:
	ADC_HandleTypeDef*	mHadc;
	uint8_t 			mNumChannels;

	uint16_t 			mDmaBuffer[ADC_CH_COUNT];



};
#endif /* APP_ADC_H_ */
