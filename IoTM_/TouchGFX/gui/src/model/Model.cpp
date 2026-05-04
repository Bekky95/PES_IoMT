#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "../../STM32CubeIDE/App/uiQueue.h"
#include "SensorHandler/SensorHandler.h"

Model::Model() : modelListener(0)
{

}

void Model::tick()
{
    SensorData data;
    //Get Semaphore and read data until queue is empty
    // TODO: check timing issues, adc could be writing here too fast and this could be blocking

	while(xQueuePeek(SensorHandler::instance().getUIQueue(), &data, 0) == pdTRUE) {
		// Read and remove item from queue
		xQueueReceive(SensorHandler::instance().getUIQueue(), &data, 0);
		modelListener->onSensorUpdated(data);
	}

}
