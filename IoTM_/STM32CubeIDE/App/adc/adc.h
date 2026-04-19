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

// Forward declaration of AdcChannel class
class AdcChannel;

class AdcDma {
public:
	friend class AdcChannel;

	explicit AdcDma(ADC_HandleTypeDef& hadc, uint8_t numChannels);
	HAL_StatusTypeDef init();

	const uint16_t* getValues() const;
	uint16_t getChannelValue(uint8_t ch);

private:
	ADC_HandleTypeDef*	mHadc;
	uint8_t 			mNumChannels;

	AdcChannel** 	    mAdcChannels;
	uint32_t* 			mDmaBuffer;



};
#endif /* APP_ADC_H_ */
