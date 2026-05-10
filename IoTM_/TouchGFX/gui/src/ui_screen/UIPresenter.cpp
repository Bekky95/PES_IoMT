#include <gui/ui_screen/UIView.hpp>
#include <gui/ui_screen/UIPresenter.hpp>

volatile uint8_t UI_READY = 0;

UIPresenter::UIPresenter(UIView& v)
    : view(v)
{

}

void UIPresenter::activate()
{
	currSensor = EMG;
	UI_READY = 1;

}

void UIPresenter::deactivate()
{

}
void UIPresenter::onSensorUpdated(const SensorData& data) {
	//TODO: update screen with new data
	//float adcData = data.adc[0];
	view.updateGraph(data);

}
