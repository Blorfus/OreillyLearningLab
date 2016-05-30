
#include "TWX2/src/twApi.h"
#include "TWX2/src/twLogger.h"
#include "TWX2/src/twOSPort.h"
#include "geoloc.h"
#include "config.h"
#include "sensors/src/sensors.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

struct {
	float temp_adt75;
	float temp_sht21;
	float rh_sht21;
	float x,y,z;
	float pressure;
	float altitude;
	float light;
	float adc[8];
} 
properties;

void *collection_thread(void *arg);
void sendPropertyUpdate();
void dataCollectionTask();
enum msgCodeEnum propertyHandler(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata);