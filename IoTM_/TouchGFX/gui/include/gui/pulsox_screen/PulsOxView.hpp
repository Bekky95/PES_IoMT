#ifndef PULSOXVIEW_HPP
#define PULSOXVIEW_HPP

#include <gui_generated/pulsox_screen/PulsOxViewBase.hpp>
#include <gui/pulsox_screen/PulsOxPresenter.hpp>
extern SensorType _activeType;

class PulsOxView : public PulsOxViewBase
{
public:
    PulsOxView();
    virtual ~PulsOxView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void updateData(const SensorData& data);
    virtual void bEMG_Clicked() override;
    virtual void bEEG_Clicked() override;
    virtual void bEKG_Clicked() override;
    void switchSource(SensorType type);

protected:

};

#endif // PULSOXVIEW_HPP
