#ifndef PULSOXVIEW_HPP
#define PULSOXVIEW_HPP

#include <gui_generated/pulsox_screen/PulsOxViewBase.hpp>
#include <gui/pulsox_screen/PulsOxPresenter.hpp>

class PulsOxView : public PulsOxViewBase
{
public:
    PulsOxView();
    virtual ~PulsOxView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void updateData(const SensorData& data);
protected:
private:
    Unicode::UnicodeChar hrBuffer[12];
    Unicode::UnicodeChar sp02Buffer[12];
};

#endif // PULSOXVIEW_HPP
