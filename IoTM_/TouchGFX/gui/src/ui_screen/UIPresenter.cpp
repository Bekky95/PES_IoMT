#include <gui/ui_screen/UIView.hpp>
#include <gui/ui_screen/UIPresenter.hpp>

UIPresenter::UIPresenter(UIView& v)
    : view(v)
{

}

void UIPresenter::activate()
{

}

void UIPresenter::deactivate()
{

}
void UIPresenter::onSensorUpdated(const SensorData& data) {
	//TODO: update screen with new data

}
