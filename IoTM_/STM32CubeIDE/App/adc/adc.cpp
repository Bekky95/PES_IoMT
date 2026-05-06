/*
 * adc.cpp
 *
 *  Created on: 12 Apr 2026
 *      Author: Lucian
 */
#include <adc/adc.h>



AdcDma::AdcDma(ADC_HandleTypeDef* hadc, uint8_t numChannels) {
	//configASSERT(numChannels == hadc->Init.NbrOfConversion);
    if (!hadc || numChannels != MAX_CHANNELS)
    {
        mHadc        = nullptr;
        mNumChannels = 0;
        return;
    }

    if (!hadc->DMA_Handle)
    {
        mHadc        = nullptr;
        mNumChannels = 0;
        return;
    }

    mHadc = hadc;
	mNumChannels = numChannels;
    memset(mDmaBuffer,    0, sizeof(mDmaBuffer));

}

HAL_StatusTypeDef AdcDma::start() {
	if(mHadc == nullptr) {
		return HAL_ERROR;
	}
	if(!mHadc->DMA_Handle) {
		return HAL_ERROR;
	}
	return HAL_ADC_Start_DMA(mHadc, mDmaBuffer, (uint32_t)mNumChannels);

}

HAL_StatusTypeDef AdcDma::stop() {
	HAL_StatusTypeDef stat =  HAL_ADC_Stop_DMA(mHadc);
	// TODO: move delete to destructor
	return stat;
}

const uint32_t* AdcDma::getValues() {
	return mDmaBuffer;
}

float AdcDma::getChannelValue(uint8_t ch) const{
	return mDmaBuffer[ch];
}

float AdcDma::GetChValVolt(uint8_t ch) const{
	//configASSERT(mAdc != nullptr);
	float max = 4096.0f;
	uint16_t rawAdc = getChannelValue(ch);
	float voltage = 0;
	voltage = (rawAdc / max) * VREF;
	return voltage;
}
