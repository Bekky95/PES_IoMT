#include <gui/pulsox_screen/PulsOxView.hpp>
#include <touchgfx/Unicode.hpp>

PulsOxView::PulsOxView()
{

}

void PulsOxView::setupScreen()
{
    PulsOxViewBase::setupScreen();
}

void PulsOxView::tearDownScreen()
{
    PulsOxViewBase::tearDownScreen();
}

void PulsOxView::updateData(const SensorData& data){
	if (data.SpO2Data.validHeartRate){
		Unicode::snprintf(heartRateTextBuffer, 12, "%d", data.SpO2Data.heartRate);
		heartRateText.setWildcard1(heartRateTextBuffer);
		heartRateText.invalidate();
	}
	if(data.SpO2Data.validSPO2){
		Unicode::snprintf(pulsOxTextBuffer, 12, "%d", data.SpO2Data.spo2);
		pulsOxText.setWildcard1(pulsOxTextBuffer);
		pulsOxText.invalidate();
	}
}

void PulsOxView::switchSource(SensorType type) {
	_activeType = type;
}

void PulsOxView::bEKG_Clicked(){
	//gData.setGraphRangeY(-0.5f, 3.5f);
	switchSource(EKG);
	SensorHandler::instance().switchTo(ADC_COMBINED);

}
void PulsOxView::bEEG_Clicked(){
	//gData.setGraphRangeY(-0.5f, 3.5f);
	switchSource(EEG);
	SensorHandler::instance().switchTo(ADC_COMBINED);
}
void PulsOxView::bEMG_Clicked(){
	//gData.setGraphRangeY(-0.5f, 3.5f);
	switchSource(EMG);
	SensorHandler::instance().switchTo(ADC_COMBINED);
}
