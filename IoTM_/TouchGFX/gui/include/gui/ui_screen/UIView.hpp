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
    virtual void invalidateGraph();
    float extractSample(const SensorData& data);
    void handleADCData(const SensorData& data);
    void bPulsOx_Hr() override;
    void bPulsOx_HR_Clicked()override;
    void bEkgClicked()override;
    void bEegClicked() override;
    void bEmgClicked() override;

private:
    void switchSource(SensorType type);

protected:
};

#endif // UIVIEW_HPP
