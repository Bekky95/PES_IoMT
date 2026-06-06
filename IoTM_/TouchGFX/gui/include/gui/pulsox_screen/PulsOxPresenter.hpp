#ifndef PULSOXPRESENTER_HPP
#define PULSOXPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class PulsOxView;

class PulsOxPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    PulsOxPresenter(PulsOxView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    void onMaxDataUpdated(const SensorData& data) override;

    virtual ~PulsOxPresenter() {}

private:
    PulsOxPresenter();

    PulsOxView& view;
};

#endif // PULSOXPRESENTER_HPP
