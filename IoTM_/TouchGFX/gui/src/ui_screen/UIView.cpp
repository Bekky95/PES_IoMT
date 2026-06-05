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
    uint16_t val = 0;
    switch (_activeType) {
        case EMG: val = data.AdcData.emgAvg; break;
        case EKG: val = data.AdcData.ekgAvg; break;
        case EEG: val = data.AdcData.eegAvg; break;
        default: return;
    }

    // add an average of the collected data points
	gData.addDataPoint(val);
	gData.invalidateContent();
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
