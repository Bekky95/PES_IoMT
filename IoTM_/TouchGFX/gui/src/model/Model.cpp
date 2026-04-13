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
    if(xQueuePeek(getSensorQueue(), &data, 0) == pdTRUE) {
    	modelListener->onSensorUpdated(data);
    }
}
