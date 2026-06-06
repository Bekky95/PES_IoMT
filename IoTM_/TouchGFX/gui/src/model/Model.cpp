#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "../../STM32CubeIDE/App/uiQueue.h"
#include "SensorHandler/SensorHandler.h"
#include "Config.h"

#define DATA_POINTS_PER_TICK	1000
extern uint8_t UI_READY;
extern osMessageQueueId_t uiQueue;
extern SensorType _activeType;


Model::Model() : modelListener(0)
{

}

void Model::tick()
{
	if(UI_READY) {
		SensorData data;
		uint8_t numDataPoints = 0;
		if(is_adc_sensor(_activeType)){
			//Get Semaphore and read data until queue is empty
			// TODO: check timing issues, adc could be writing here too fast and this could be blocking
			while(osMessageQueueGet(uiQueue, &data, 0, 0) == osOK && data.type == ADC_COMBINED) {
				// Read and remove item from queue
				modelListener->onSensorUpdated(data);
				numDataPoints++;

			}
		} else if(is_max_sensor(_activeType)){
			while(osMessageQueueGet(uiQueue, &data, 0, 0) == osOK && data.type == MAX1030x){
				modelListener->onMaxDataUpdated(data);
				numDataPoints++;
			}
		}
		//modelListener->invalidateGraph();
	}

}
