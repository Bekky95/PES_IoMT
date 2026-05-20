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
	if((is_adc_sensor(data.type) && is_adc_sensor(_activeType))|| (is_max_sensor(data.type) && is_max_sensor(_activeType))) {
		float val = extractSample(data);
		gData.addDataPoint(val);
	}

}
void UIView::invalidateGraph() {
	gData.invalidate();
}
//TODO: fix sensor type handling here:
float UIView::extractSample(const SensorData& data){
    switch (_activeType)
    {
        case EMG:      return data.AdcData.emg;
        case EEG:      return data.AdcData.eeg;
        case EKG:      return data.AdcData.ekg;
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
