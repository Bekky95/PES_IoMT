/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : app_freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SensorHandler/SensorHandlerConfig.h"
#include "queue.h"
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;

#ifdef __cplusplus
extern "C" {
#endif

extern void SensorHandler_Start(SensorHandlerConfig *config,
		const osThreadAttr_t *attr);

extern BaseType_t sP02Init(SpO2Config cfg);

// ADC Task:
extern BaseType_t adcInit(adcConfig cfg);
extern void* adcHandlerGetInstance(void);
extern void ADCHandler_TaskEntry(void* arg);

extern const QueueHandle_t getSensorQueue(void);
#ifdef __cplusplus
}
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* Definitions for tSensorHandler */
osThreadId_t tSensorHandlerHandle;
const osThreadAttr_t tSensorHandler_attributes = { .name = "tSensorHandler",
		.priority = (osPriority_t) osPriorityNormal, .stack_size = 128 * 4 };

osMessageQueueId_t sp02_to_SensorHandlerHandle;
const osMessageQueueAttr_t sp02_to_SensorHandler_attributes = { .name =
		"sp02_SensorHandler_Queue" };

osMessageQueueId_t adc_to_SensorHandlerHandle;
const osMessageQueueAttr_t adc_to_SensorHandler_attributes = { .name =
		"adc_SensorHandler_Queue" };

osMessageQueueId_t uiQueue;
const osMessageQueueAttr_t uiQueueAttributes = { .name = "uiQueue" };

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
		.priority = (osPriority_t) osPriorityLow4, .stack_size = 128 * 4 };
/* Definitions for GUI_Task */
osThreadId_t GUI_TaskHandle;
const osThreadAttr_t GUI_Task_attributes = { .name = "GUI_Task", .priority =
		(osPriority_t) osPriorityNormal, .stack_size = 8192 * 4 };
/* Definitions for sp02Task */
osThreadId_t sp02TaskHandle;
const osThreadAttr_t sp02Task_attributes = { .name = "sp02Task", .priority =
		(osPriority_t) osPriorityNormal, .stack_size = 512 * 4 };
/* Definitions for adcSensorsTask */
osThreadId_t adcSensorsTaskHandle;
const osThreadAttr_t adcSensorsTask_attributes = { .name = "adcSensorsTask",
		.priority = (osPriority_t) osPriorityHigh, .stack_size = 128 * 4 };
/* Definitions for UIQueueSem */
osSemaphoreId_t UIQueueSemHandle;
const osSemaphoreAttr_t UIQueueSem_attributes = { .name = "UIQueueSem" };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern portBASE_TYPE IdleTaskHook(void *p);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
extern void TouchGFX_Task(void *argument);
void startSp02(void *argument);
void StartAdcSensors(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook(void) {
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	 to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
	 task. It is essential that code added to this hook function never attempts
	 to block in any way (for example, call xQueueReceive() with a block time
	 specified, or call vTaskDelay()). If the application makes use of the
	 vTaskDelete() API function (as this demo application does) then it is also
	 important that vApplicationIdleHook() is permitted to return to its calling
	 function, because it is the responsibility of the idle task to clean up
	 memory allocated by the kernel to any task that has since been deleted. */

	vTaskSetApplicationTaskTag(NULL, IdleTaskHook);
}
/* USER CODE END 2 */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */
	/* creation of UIQueueSem */
	UIQueueSemHandle = osSemaphoreNew(1, 1, &UIQueueSem_attributes);

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	uiQueue = osMessageQueueNew(20, sizeof(SensorData), &uiQueueAttributes);

	sp02_to_SensorHandlerHandle = osMessageQueueNew(16, sizeof(uint16_t),
			&sp02_to_SensorHandler_attributes);
	/* creation of adc_to_SensorHandler */
	adc_to_SensorHandlerHandle = osMessageQueueNew(16, sizeof(uint16_t),
			&adc_to_SensorHandler_attributes);

	// has to be placed here otherwise it will be erased with generating new code
	SpO2Config sP02_Config;
	sP02_Config.queue = sp02_to_SensorHandlerHandle;
	sP02_Config.hi2c = &hi2c1;


	adcConfig adc_Config;
	adc_Config.queue = adc_to_SensorHandlerHandle;
	adc_Config.adc = &hadc1;
	if(!adcInit(adc_Config)) Error_Handler();
	/* USER CODE END RTOS_QUEUES */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

	/* creation of GUI_Task */
	GUI_TaskHandle = osThreadNew(TouchGFX_Task, NULL, &GUI_Task_attributes);

	/* creation of sp02Task */
	sp02TaskHandle = osThreadNew(startSp02, &sP02_Config, &sp02Task_attributes);

	/* creation of adcSensorsTask */
	adcSensorsTaskHandle = osThreadNew(StartAdcSensors, adcHandlerGetInstance(),
			&adcSensorsTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	SensorHandlerConfig config = { .hadc = &hadc1, .adcChannelCount = 1, .hi2c =
			&hi2c1, .i2cAddress = 0x48, .i2cReadBytes = 2, .loopPeriodMs = 20,
			.uiQueue = uiQueue, .uiSem = UIQueueSemHandle, };

	SensorHandler_Start(&config, &tSensorHandler_attributes);
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief Function implementing the defaultTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument) {
	/* USER CODE BEGIN defaultTask */

	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
	/* USER CODE END defaultTask */
}

/* USER CODE BEGIN Header_startSp02 */
/**
 * @brief Function implementing the sp02Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_startSp02 */
void startSp02(void *argument) {
	/* USER CODE BEGIN sp02Task */
	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
	/* USER CODE END sp02Task */
}

/* USER CODE BEGIN Header_StartAdcSensors */
/**
 * @brief Function implementing the adcSensorsTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartAdcSensors */
void StartAdcSensors(void *argument) {
	/* USER CODE BEGIN adcSensorsTask */
	/* Infinite loop */
	ADCHandler_TaskEntry(argument);
	// Should never land here!!!
	for (;;){
		osDelay(1);
	}
	/* USER CODE END adcSensorsTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
QueueHandle_t getSensorQueue(void) {
	return uiQueue;
}
/* USER CODE END Application */

