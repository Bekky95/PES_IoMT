/*
 * adc.cpp
 *
 *  Created on: 12 Apr 2026
 *      Author: Lucian
 */
#include <adc/adc.h>
#include <cstring>

AdcDma::AdcDma(ADC_HandleTypeDef* hadc, uint8_t numChannels) {
	mHadc = hadc;
	mNumChannels = numChannels;
}

HAL_StatusTypeDef AdcDma::start() {
	return HAL_ADC_Start_DMA(mHadc, mDmaBuffer, mNumChannels);
}

HAL_StatusTypeDef AdcDma::stop() {
	return HAL_ADC_Stop_DMA(mHadc);
}

const uint32_t* AdcDma::getValues() {
	return mDmaBuffer;
}

uint32_t AdcDma::getChannelValue(uint8_t ch) const{
	return mDmaBuffer[ch];
}

AdcChannel* AdcDma::registerChannel(uint8_t ch) const {
	AdcChannel* ret = new AdcChannel(this, ch);
	//TODO add new channel to list of channel
	//this->mAdcChannels[ch] = ret;
	return ret;
}
