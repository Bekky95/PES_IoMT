/*
 * uiQueue.h
 *
 *  Created on: 13 Apr 2026
 *      Author: Lucian
 */

#ifndef APP_UIQUEUE_H_
#define APP_UIQUEUE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "queue.h"

const QueueHandle_t getSensorQueue(void);

#ifdef __cplusplus
}
#endif


#endif /* APP_UIQUEUE_H_ */
