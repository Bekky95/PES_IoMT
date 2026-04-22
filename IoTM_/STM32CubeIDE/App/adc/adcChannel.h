/*
 * adcChannel.h
 *
 *  Created on: 15 Apr 2026
 *      Author: Lucian
 */

#ifndef APP_ADC_ADCCHANNEL_H_
#define APP_ADC_ADCCHANNEL_H_
#include "main.h"
#include <stdint.h>

static const float VREF = 3.3f;

class AdcDma;

class AdcChannel {
public:
	AdcChannel(const AdcDma* adc ,uint8_t index);
	uint32_t getValue();
	float getVoltValue();

private:
	const AdcDma*	mAdc;
	uint8_t				mIndex;
};


#endif /* APP_ADC_ADCCHANNEL_H_ */
