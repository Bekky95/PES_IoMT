#ifndef UIPRESENTER_HPP
#define UIPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class UIView;

class UIPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    UIPresenter(UIView& v);

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

    virtual ~UIPresenter() {}

private:
    UIPresenter();

    UIView& view;
};

#endif // UIPRESENTER_HPP
