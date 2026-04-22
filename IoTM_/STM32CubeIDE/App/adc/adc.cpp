/*
 * adc.cpp
 *
 *  Created on: 12 Apr 2026
 *      Author: Lucian
 */
#include <adc/adc.h>
#include <cstring>


AdcDma::AdcDma(ADC_HandleTypeDef* hadc, uint8_t numChannels) {
	configASSERT(numChannels == hadc->Init.NbrOfConversion);
	mHadc = hadc;
	mNumChannels = numChannels;
	mDmaBuffer = new uint32_t[mNumChannels];
	memset(mDmaBuffer, 0, sizeof(uint32_t) * mNumChannels);
	mAdcChannels = new AdcChannel*[mNumChannels];
	memset(mAdcChannels, 0, sizeof(AdcChannel*) * mNumChannels);
}

HAL_StatusTypeDef AdcDma::start() {
	return HAL_ADC_Start_DMA(mHadc, mDmaBuffer, (uint32_t)mNumChannels);
}

HAL_StatusTypeDef AdcDma::stop() {
	HAL_StatusTypeDef stat =  HAL_ADC_Stop_DMA(mHadc);
	// TODO: move delete to destructor
	delete[] mDmaBuffer;
	return stat;
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
