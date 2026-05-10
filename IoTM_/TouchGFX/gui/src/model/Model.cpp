#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "../../STM32CubeIDE/App/uiQueue.h"
#include "SensorHandler/SensorHandler.h"
extern osMessageQueueId_t uiQueue;
Model::Model() : modelListener(0)
{

}

void Model::tick()
{
    SensorData data;
    //Get Semaphore and read data until queue is empty
    // TODO: check timing issues, adc could be writing here too fast and this could be blocking
    uint32_t cnt = osMessageQueueGetCount(uiQueue);
	while(cnt > 0) {
		// Read and remove item from queue
		if(osMessageQueueGet(uiQueue, &data, 0, 0) == osOK ) {
			//	xQueueReceive(SensorHandler::instance().getUIQueue(), &data, 0);
			modelListener->onSensorUpdated(data);

		}
	}

}
