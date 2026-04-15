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
#include "adcChannel.h"

class AdcDma {
public:
	// Static Variables
	static constexpr uint8_t CHANNEL_COUNT = 1;

	AdcDma(ADC_HandleTypeDef* hadc, uint8_t numChannels);

	HAL_StatusTypeDef start();
	HAL_StatusTypeDef stop();

	const uint32_t* getValues();
	uint32_t getChannelValue(uint8_t ch) const;

	AdcChannel* registerChannel(uint8_t ch) const;

private:
	ADC_HandleTypeDef*	mHadc;
	uint8_t 			mNumChannels;

	AdcChannel* 	    mAdcChannels[CHANNEL_COUNT];
	uint32_t 			mDmaBuffer[CHANNEL_COUNT];


};
#endif /* APP_ADC_H_ */
