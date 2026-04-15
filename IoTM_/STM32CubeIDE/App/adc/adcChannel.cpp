/*
 * adcChannel.cpp
 *
 *  Created on: 15 Apr 2026
 *      Author: Lucian
 */
#include "adcChannel.h"
#include "adc.h"

AdcChannel::AdcChannel(const AdcDma* adc ,uint8_t index) {
	mAdc = adc;
	mIndex = index;
}

uint16_t AdcChannel::getValue() {
	return mAdc->getChannelValue(mIndex);
}


