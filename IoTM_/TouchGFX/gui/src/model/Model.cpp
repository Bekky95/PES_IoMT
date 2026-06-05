#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "../../STM32CubeIDE/App/uiQueue.h"
#include "SensorHandler/SensorHandler.h"

#define DATA_POINTS_PER_TICK	1000
extern uint8_t UI_READY;
extern osMessageQueueId_t uiQueue;


Model::Model() : modelListener(0)
{

}

void Model::tick()
{
	if(UI_READY) {
		SensorData data;
		uint8_t numDataPoints = 0;
		//Get Semaphore and read data until queue is empty
		// TODO: check timing issues, adc could be writing here too fast and this could be blocking
		while(osMessageQueueGet(uiQueue, &data, 0, 0) == osOK ) {
			// Read and remove item from queue
			modelListener->onSensorUpdated(data);
			numDataPoints++;

		}
		//modelListener->invalidateGraph();
	}

}
