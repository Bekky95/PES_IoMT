#ifndef UIVIEW_HPP
#define UIVIEW_HPP

#include <gui_generated/ui_screen/UIViewBase.hpp>
#include <gui/ui_screen/UIPresenter.hpp>

class UIView : public UIViewBase
{
public:
    UIView();
    virtual ~UIView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void updateGraph(SensorData data);
    float extractSample(const SensorData& data);
    void bPulsOxClicked() override;
    void bEkgClicked()override;
    void bEegClicked() override;
    void bEmgClicked() override;
private:
    void switchSource(SensorType type);
    SensorType _activeType = EMG;
protected:
};

#endif // UIVIEW_HPP
