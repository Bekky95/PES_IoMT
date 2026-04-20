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

uint32_t AdcChannel::getValue() {
	return mAdc->getChannelValue(mIndex);
}
float AdcChannel::getVoltValue() {
	configASSERT(mAdc != nullptr);
	float max = 4096.0f;
	uint16_t rawAdc = mAdc->getChannelValue(mIndex);
	float voltage = 0;
	voltage = (rawAdc / max) * VREF;
	return voltage;
}


