#include <gui/ui_screen/UIView.hpp>

volatile SensorType _activeType = EMG;

UIView::UIView()
{

}

void UIView::setupScreen()
{
    UIViewBase::setupScreen();
}

void UIView::tearDownScreen()
{
    UIViewBase::tearDownScreen();
}

void UIView::updateGraph(SensorData data)
{
	if((is_max_sensor(data.type) && is_max_sensor(_activeType))) {
		float val = extractSample(data);
		gData.addDataPoint(val);
	} else if ( (is_adc_sensor(data.type) && is_adc_sensor(_activeType))){
		handleADCData(data);
	}

}
void UIView::handleADCData(const SensorData& data) {
    uint8_t chOffset = 0;
    switch (_activeType) {
        case EMG: chOffset = 0; break;
        case EKG: chOffset = 1; break;
        case EEG: chOffset = 2; break;
        default: return;
    }

    for (uint32_t i = 0; i < data.AdcData.count; i++) {
        int16_t sample = (int16_t)data.AdcData.values[i * ADC_CH_COUNT + chOffset];
        gData.addDataPoint(sample);
		gData.invalidateContent();
    }
}
void UIView::invalidateGraph() {
	gData.invalidate();
}

float UIView::extractSample(const SensorData& data){
    switch (_activeType)
    {
        case MAX_HR:   return (float)data.SpO2Data.heartRate;
        case MAX_Sp02: return (float)data.SpO2Data.spo2;
        default:       return 0.0f;
    }
}
void UIView::switchSource(SensorType type) {
	_activeType = type;
	gData.clear();

}
void UIView::bPulsOx_Hr() {
	gData.setGraphRangeY(-10, 200);
	switchSource(MAX_HR);
}
void UIView::bPulsOx_HR_Clicked(){
	gData.setGraphRangeY(-10, 110);
	switchSource(MAX_Sp02);
}

void UIView::bEkgClicked(){
	//gData.setGraphRangeY(-0.5f, 3.5f);
	switchSource(EKG);
}
void UIView::bEegClicked(){
	//gData.setGraphRangeY(-0.5f, 3.5f);
	switchSource(EEG);
}
void UIView::bEmgClicked(){
	//gData.setGraphRangeY(-0.5f, 3.5f);
	switchSource(EMG);
}
