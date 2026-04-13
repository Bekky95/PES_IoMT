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

class AdcDma {
public:
	// Static Variables
	static constexpr uint8_t CHANNEL_COUNT = 3;

	explicit AdcDma(ADC_HandleTypeDef& hadc, uint8_t numChannels);
	HAL_StatusTypeDef init();

	const uint16_t* getValues() const;
	uint16_t getChannelValue(uint8_t ch);

private:
	ADC_HandleTypeDef*	mHadc;
	uint8_t 			mNumChannels;

	uint16_t 			mDmaBuffer[CHANNEL_COUNT];


};
#endif /* APP_ADC_H_ */
