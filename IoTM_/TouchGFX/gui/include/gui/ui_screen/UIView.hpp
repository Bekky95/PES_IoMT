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
    virtual void updateGraph(float val);
protected:
};

#endif // UIVIEW_HPP
