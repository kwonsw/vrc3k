/**
 *	@file   mutex.h
 *	@brief	
 *	@author malas <malas@pinetron.com>
 *	@date   2015/06/30 10:29
 */

#ifndef _MUTEX_HEADER_
#define _MUTEX_HEADER_

/* system include */
#include <FreeRTOS.h>
#include <semphr.h>

/* local include */

/* external variable & function */
#define MUTEX_DECLARE(sem)		static SemaphoreHandle_t sem = NULL
#define MUTEX_INIT(sem)			do { sem = xSemaphoreCreateRecursiveMutex(); } while(0)
#define MUTEX_LOCK(sem)			xSemaphoreTakeRecursive(sem, portMAX_DELAY)
#define MUTEX_UNLOCK(sem)		xSemaphoreGiveRecursive(sem)

#endif /* _MUTEX_HEADER_*/

