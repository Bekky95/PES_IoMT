#include <gui/pulsox_screen/PulsOxView.hpp>
#include <gui/pulsox_screen/PulsOxPresenter.hpp>

PulsOxPresenter::PulsOxPresenter(PulsOxView& v)
    : view(v)
{

}

void PulsOxPresenter::activate()
{

}

void PulsOxPresenter::deactivate()
{

}

void PulsOxPresenter::onMaxDataUpdated(const SensorData& data){
	view.updateData(data);
}
