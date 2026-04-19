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
    //Check if item in queue is not nullptr without removing it from queue
    if(xQueuePeek(SensorHandler::instance().getUIQueue(), &data, 0) == pdTRUE) {
    	// Read and remove item from queue
    	xQueueReceive(SensorHandler::instance().getUIQueue(), &data, 0);
    	modelListener->onSensorUpdated(data);
    }
}
