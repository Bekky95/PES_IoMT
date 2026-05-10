#include <gui/ui_screen/UIView.hpp>

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
	if(_activeType == data.type) {
		float val = extractSample(data);
		gData.addDataPoint(val);
		gData.invalidate();
	}

}

float UIView::extractSample(const SensorData& data){
    switch (data.type)
    {
        case EMG:  return (float)data.EmgData;
        case EEG:  return (float)data.EegData;
        case EKG:  return (float)data.EkgData;
        case MAX1030x: return (float)data.SpO2Data.spo2;
        default:          return 0;
    }
}
void UIView::switchSource(SensorType type) {
	_activeType = type;
	gData.clear();

}
void UIView::bPulsOxClicked() {
	switchSource(MAX1030x);
}
void UIView::bEkgClicked(){switchSource(EKG);}
void UIView::bEegClicked() {switchSource(EEG);}
void UIView::bEmgClicked() {switchSource(EMG);}
