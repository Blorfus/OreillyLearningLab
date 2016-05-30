/*
Copyright (c) 2015 PTC Inc. 
Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define DEBUG

#define TEMPLATE "FAEBoardTemplate"

#include "main.h"

config_t	cfg;

i2c_t 		i2c;

adc_t		s_adc;
adt75_t		s_temp;
mpl_t 		s_baro;
sht21_t 	s_temprh;
adxl_t 		s_accel;
tsl_t 		s_lux;

location_t loc;
struct twLocation twloc;

void sendPropertyUpdate() {
	twApi_WriteProperty(TW_THING, cfg.name, "Temp_ADT75", twPrimitive_CreateFromNumber(properties.temp_adt75), 0, FALSE);
	twApi_WriteProperty(TW_THING, cfg.name, "Temp_SHT21", twPrimitive_CreateFromNumber(properties.temp_sht21), 0, FALSE);
	twApi_WriteProperty(TW_THING, cfg.name, "Humidity", twPrimitive_CreateFromNumber(properties.rh_sht21), 0, FALSE);
	
	twApi_WriteProperty(TW_THING, cfg.name, "X", twPrimitive_CreateFromNumber(properties.x), 0, FALSE);
	twApi_WriteProperty(TW_THING, cfg.name, "Y", twPrimitive_CreateFromNumber(properties.y), 0, FALSE);
	twApi_WriteProperty(TW_THING, cfg.name, "Z", twPrimitive_CreateFromNumber(properties.z), 0, FALSE);
	
	twApi_WriteProperty(TW_THING, cfg.name, "Pressure", twPrimitive_CreateFromNumber(properties.pressure), 0, FALSE);
	twApi_WriteProperty(TW_THING, cfg.name, "Altitude", twPrimitive_CreateFromNumber(properties.altitude), 0, FALSE);
	
	twApi_WriteProperty(TW_THING, cfg.name, "Light", twPrimitive_CreateFromNumber(properties.light), 0, FALSE);
		
	int x = 0;
	char adcname[5];
	for(x=0;x<8;x++) {
		sprintf(adcname, "ADC%d", x+1);
		twApi_WriteProperty(TW_THING, cfg.name, adcname, twPrimitive_CreateFromNumber(properties.adc[x]), 0, FALSE);
	}
}

void dataCollectionTask() {
	
	static int counter = 0;
	
	adt75_measure(&s_temp);
	mpl_measure(&s_baro);
	sht21_measure(&s_temprh);
	adxl_measure(&s_accel);
	tsl_measure(&s_lux);
	adc_measure(&s_adc);
	
	properties.temp_adt75 = s_temp.temperature*(9.0/5.0)+32.0;
	properties.temp_sht21 = s_temprh.temperature_f;
	properties.rh_sht21 = s_temprh.humidity;
	properties.pressure = s_baro.pressure * 0.000295301f;
	properties.altitude = s_baro.altitude;
	properties.light = s_lux.lux;
	properties.x = s_accel.x;
	properties.y = s_accel.y;
	properties.z = s_accel.z;

	int x = 0;
	for(x=0;x<8;x++)
		properties.adc[x] = s_adc.volts[x];
	
	printf("[%d\t] Data Collected!",counter++);
	
	sendPropertyUpdate();
}

enum msgCodeEnum propertyHandler(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) {

}

int main() {
	
	TW_LOG(TW_INFO,"Starting up!");
	
	printf("Analog Devices IoT ThingWorx Demo\n");
	
	//Clear out the properties struct
	memset(&properties, 0, sizeof(properties));
	
	/* Load configuration */
	if(config_read(&cfg)==0) {
		printf("\nNo configuration file loaded. Please write a configuration file to /boot/iot_config.txt\n");
		return 1;
	}
	
	/* Set up sensors and ports */
	i2c_init(&i2c, "/dev/i2c-1");
	
	adt75_init(&s_temp, &i2c, 0x48);
	mpl_init(&s_baro, &i2c, 0x60);
	sht21_init(&s_temprh, &i2c, 0x40);
	adxl_init(&s_accel);
	tsl_init(&s_lux, &i2c, 0x29);
	adc_init(&s_adc, &i2c, 0x10);
		
	int err=0;
	
	/* Set up Thing */
	config_print(&cfg);
	
	if(!checkIfThingExists(cfg.name,cfg.instance, cfg.appkey)) {
		printf("Creating new Thing.\n");
		createThingFromTemplate(cfg.name, TEMPLATE, cfg.instance, cfg.appkey);
	}
	
	err = twApi_Initialize(cfg.instance, 80, TW_URI, cfg.appkey, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	if (err) {
		printf("Error initializing the API\n");
		return 1;
	}
	
	/* Allow self signed certs */
	twApi_SetSelfSignedOk();
	
	
	/* Connect to server */
	if (!twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES)) {
		printf("Successfully connected to TWX!\n");
	}
	
	/* Report location */
	
	twloc.latitude = strtof(cfg.lat, NULL);
	twloc.longitude = strtof(cfg.lng, NULL);
	twApi_WriteProperty(TW_THING, cfg.name, "Location", twPrimitive_CreateFromLocation(&twloc), 0, FALSE);
	
	printf("\n");
	
	/* Start a thread to collect data at an interval */
	pthread_t thread = 0;
	int error = pthread_create(&thread, NULL, &collection_thread, NULL);
	if(error < 0) {
		//An error occured.
		printf("Could not begin data collection! Thread Error %d.\n",error);
	}
	
	/* Don't end the program while the collection thread is running. */
	pthread_join(thread, NULL);
	
	return 0;
}

void *collection_thread(void *arg) {
	
	struct timeval start;
	struct timeval end;
	long difference;
	
	while(1) {
		gettimeofday(&start, NULL);
		
		//Take Measurements and send them.
		dataCollectionTask();
		
		gettimeofday(&end, NULL);
		
		difference = (end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000+start.tv_usec);
		if(difference >= 0) {
			usleep(5*1000000-difference);
		}
	}
}



