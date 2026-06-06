#include <gui/pulsox_screen/PulsOxView.hpp>
#include <touchgfx/Unicode.hpp>

PulsOxView::PulsOxView()
{

}

void PulsOxView::setupScreen()
{
    PulsOxViewBase::setupScreen();
}

void PulsOxView::tearDownScreen()
{
    PulsOxViewBase::tearDownScreen();
}

void PulsOxView::updateData(const SensorData& data){
	if (data.SpO2Data.validHeartRate){
		Unicode::snprintf(hrBuffer, 12, "%d", data.SpO2Data.heartRate);
		heartRateText.setWildcard1(hrBuffer);
		heartRateText.invalidate();
	}
	if(data.SpO2Data.validSPO2){
		Unicode::snprintf(sp02Buffer, 12, "%d", data.SpO2Data.spo2);
		pulsOxText.setWildcard1(sp02Buffer);
		pulsOxText.invalidate();
	}
}
