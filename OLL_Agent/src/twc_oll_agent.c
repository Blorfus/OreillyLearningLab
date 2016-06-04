// Standard includes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

//App Specific includes
#include "Settings.h"
#include "pinspectre.h"
#include "pinspectre_hwbase.h"

//FAE Board Includes
#include "fae/i2c.h"
#include "fae/adt75.h"
#include "fae/mpl.h"
#include "fae/sht21.h"
#include "fae/adxl362.h"
#include "fae/tsl2561.h"
#include "fae/adc.h"
#include "geoloc.h"

//ThingWorx CSDK includes
#include "twLinux.h"
#include "twLogger.h"
#include "twApi.h"

//*****************************************************************************
//                          MACROS
//*****************************************************************************

#define APPLICATION_NAME        "O'Reilly Learning Lab Agent"
#define APPLICATION_VERSION     "1.0.0"

#define PIN_ENABLE_PULLUP   1
#define PIN_DISABLE_PULLUP  0
#define PIN_LBL_ANALOG	    "A"
#define PIN_LBL_DIGITAL	    "D"
#define PIN_LBL_INPUT	    "I"
#define PIN_LBL_OUTPUT	    "O"

//*****************************************************************************
//                 STRUCTS -- Start
//*****************************************************************************

typedef struct{
    int direction;
    int type;
    int value;
}pinConfig;

/*****************
A simple structure to handle properties. Not related to
the API in anyway, just for the demo application.
******************/
typedef struct{
	float temp_adt75;
	float temp_sht21;
	float humidity;
	float x,y,z;
	float barometer;
	float altitude;
	float light;
	float adc[8];
	char * hostName;
    char * IPAddress;
    struct twLocation Location;
}local_properties;

//FAE constructs
	i2c_t		i2c;
	adc_t		s_adc;
	adt75_t		s_temp;
	mpl_t		s_baro;
	sht21_t		s_temprh;
	adxl_t		s_accel;
	tsl_t		s_lux;
//External Location struct
	location_t  currentLoc;
	
//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
unsigned long g_ulStatus=0;
tw_settings progSets;
propertyList *g_pinConf=NULL;
local_properties properties;

#if defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************




//***********************************************************************************//
//                             queryFAE()                                            //
//                                                                                   //
//Accesses the FAE board's sensors and stores the retrieved value in a local struct  //
//If unit changes or other calculations need to be performed, this is the place.     //
//***********************************************************************************//
void queryFAE(){
	
	adt75_measure(&s_temp);
	mpl_measure(&s_baro);
	
	adxl_measure(&s_accel);
	tsl_measure(&s_lux);
	adc_measure(&s_adc);
	
	properties.temp_adt75 = s_temp.temperature*(9.0/5.0)+32.0;
	
	properties.humidity = s_temprh.humidity;
	properties.barometer = s_baro.pressure * 0.000295301f;
	properties.altitude = s_baro.altitude;
	properties.light = s_lux.lux;
	properties.x = s_accel.x;
	properties.y = s_accel.y;
	properties.z = s_accel.z;

	int x = 0;
	for(x=0;x<8;x++)
		properties.adc[x] = s_adc.volts[x];
	
	printf("FAE: T1: %f, T2: %f, Humidity: %f, Accelerometer:[%f,%f,%f], Pressure: %f, Altitude: %f, Light: %f, ADC:[%f,%f,%f,%f,%f,%f,%f,%f]\n", properties.temp_adt75, properties.temp_sht21, properties.humidity, properties.x, properties.y, properties.z, properties.barometer, properties.altitude, properties.light, properties.adc[0],properties.adc[1],properties.adc[2],properties.adc[3],properties.adc[4],properties.adc[5],properties.adc[6],properties.adc[7]);
	
}


//***********************************************************************************//
//                             writeProperty()                                       //
//                                                                                   //
//This function is called automatically whenever a value is set for a property by the//
//ThingWorx Platform. It consists of a section of if else statements that further    //
// direct what happens, if anything, when a property is written*                     //
//***********************************************************************************//
enum msgCodeEnum writeProperty(const char * propertyName, twInfoTable * value) {
	if (!propertyName || !value) return TWX_BAD_REQUEST;
	printf("Got VALUE FROM DI: %s\n", propertyName);

	if(strstr(propertyName, "Config")!=NULL) {
	    char *pinCmd;    
	    twInfoTable_GetString(value, propertyName,0, &pinCmd);
	    
	  //  Store pinConfig text in the JSON and set the pin
	    printf("SETTING VALUE FROM DI: %s\n", propertyName);
	    setPinFromConfStr((char *)propertyName, pinCmd);
	     }
	   else if(strstr(propertyName, "GPIO")!=NULL)  {
	     double int_val=-1;
	     twInfoTable_GetNumber(value, propertyName, 0, &int_val);
	     setValueFromPinName((char *)propertyName, (int)int_val);
	   }
return TWX_SUCCESS;
}


//***********************************************************************************//
//                             sendPropertyUpdate()                                  //
//                                                                                   //
//This function reads the values from the local struct "properties" and adds         //
//them to a list of properties that it will send up to the ThingWorx platform        //
//When it is done adding the values it will send the data.                           //
//***********************************************************************************//
void sendPropertyUpdate(propertyList *inputProplist) {
	//We start by getting the latest data from the FAE sensors
	printf("Querying FAE sensors...\n");
	queryFAE();
	//We declare a propertyList to hold all our new property values
    propertyList *proplist=NULL;
printf("creating property list....%p", inputProplist);
    	if (!inputProplist) {
		proplist = twApi_CreatePropertyList("Hostname",twPrimitive_CreateFromString(properties.hostName, 1), 0);
		}
	else {
	     proplist=inputProplist;
	     twApi_AddPropertyToList(proplist, "HostName", twPrimitive_CreateFromString(properties.hostName, 1), 0);
	     }
	printf("Added Hostname...%p\n", properties.hostName);
    //We start adding all of our readings to the property list....
	//FAE I2C Sensors
	twApi_AddPropertyToList(proplist,"temperature_adt75",twPrimitive_CreateFromNumber(properties.temp_adt75), 0);
	
	twApi_AddPropertyToList(proplist,"humidity",twPrimitive_CreateFromNumber(properties.humidity), 0);
	twApi_AddPropertyToList(proplist,"barometer",twPrimitive_CreateFromNumber(properties.barometer), 0);
	twApi_AddPropertyToList(proplist,"altitude",twPrimitive_CreateFromNumber(properties.altitude), 0);
	twApi_AddPropertyToList(proplist,"light",twPrimitive_CreateFromNumber(properties.light), 0);
	twApi_AddPropertyToList(proplist,"accel_x",twPrimitive_CreateFromNumber(properties.x), 0);
	twApi_AddPropertyToList(proplist,"accel_y",twPrimitive_CreateFromNumber(properties.y), 0);
	twApi_AddPropertyToList(proplist,"accel_z",twPrimitive_CreateFromNumber(properties.z), 0);
	
	//FAE Analog ADC Pins	
	twApi_AddPropertyToList(proplist,"ADC1",twPrimitive_CreateFromNumber(properties.adc[0]), 0);
	twApi_AddPropertyToList(proplist,"ADC2",twPrimitive_CreateFromNumber(properties.adc[1]), 0);
	twApi_AddPropertyToList(proplist,"ADC3",twPrimitive_CreateFromNumber(properties.adc[2]), 0);
	twApi_AddPropertyToList(proplist,"ADC4",twPrimitive_CreateFromNumber(properties.adc[3]), 0);
	twApi_AddPropertyToList(proplist,"ADC5",twPrimitive_CreateFromNumber(properties.adc[4]), 0);
	twApi_AddPropertyToList(proplist,"ADC6",twPrimitive_CreateFromNumber(properties.adc[5]), 0);
	twApi_AddPropertyToList(proplist,"ADC7",twPrimitive_CreateFromNumber(properties.adc[6]), 0);
	twApi_AddPropertyToList(proplist,"ADC8",twPrimitive_CreateFromNumber(properties.adc[7]), 0);
	
	
	twApi_AddPropertyToList(proplist,"IPAddress",twPrimitive_CreateFromString(properties.IPAddress, 1), 0);
	printf("Added IP Address...%p\n", properties.IPAddress);
	printf("Staring Board Specific properties...%p\n", proplist);
	getTWPropertyUpdates(proplist); //send up board specific properties with current readings
	getTWPinConfigs(proplist);	
	twApi_AddPropertyToList(proplist,"Location",twPrimitive_CreateFromLocation(&properties.Location), 0);
	//Now that we've added all our properties, we need to send them up to the server.
	if (twApi_PushProperties(TW_THING, progSets.tw_name, proplist, -1, FALSE) != TW_OK) {
        // FIXME: need to print the error message
        printf("Failed to push properties.\n");

    }
	//Keepin it clean, no lost memory here...
	twApi_DeletePropertyList(proplist);
}




//***********************************************************************************//
//                             propertyHandler()                                     //
//                                                                                   //
//This function is called when the server sends a request for a property write or a  //
//property read. Property write is exactly as it sounds, the server has a new value  //
//for the property and it should be set here in the software. In the case of a pin   //
//it could be a new state(HIGH/LOW). Property reads on the other hand are a mechanism//
//to get the latest value for a specific property, like a direct query.              //
//***********************************************************************************//
enum msgCodeEnum propertyHandler(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) {
	char * asterisk = "*";
	if (!propertyName) propertyName = asterisk;
	printf("propertyHandler - Function called for Entity %s, Property %s\r\n", entityName, propertyName);
	TW_LOG(TW_TRACE,"propertyHandler - Function called for Entity %s, Property %s", entityName, propertyName);
	if (value) {
		if (isWrite && *value) {
			/* Property Writes */
			/* All are writable */
			return writeProperty(propertyName, *value);
		} else {
			/* Property Reads */
			if (strcmp(propertyName, "Name") == 0) *value = twInfoTable_CreateFromString(propertyName, properties.hostName, TRUE);
            		else if (strcmp(propertyName, "IPAddress") == 0) *value = twInfoTable_CreateFromString(propertyName, properties.IPAddress, TRUE);
			else if (strcmp(propertyName, "Location") == 0) *value = twInfoTable_CreateFromLocation(propertyName, &properties.Location);

			else return TWX_NOT_FOUND;
		}
		return TWX_SUCCESS;
	} else {
		TW_LOG(TW_ERROR,"propertyHandler - NULL pointer for value");
		return TWX_BAD_REQUEST;
	}
}

//**********************************************************************************//
//                             resetTask()                                          //
//                                                                                  //
//This function is invoked when the resetTask service is called from the server. It //
//will stop the agent from executing and perform a clean exit.                      //
//**********************************************************************************//
void resetTask(DATETIME now, void * params) {
	TW_LOG(TW_FORCE,"shutdownTask - Shutdown service called.  SYSTEM IS SHUTTING DOWN");
	twApi_UnbindThing(progSets.tw_name);
	twSleepMsec(100);
	twApi_Delete();
	twLogger_Delete();
}

//**********************************************************************************//
//                             multiServiceHandler()                                //
//                                                                                  //
//This function acts as a catchall for multiple services that could be called from  //
//the server. In this particular case we are only responding to the Reset service.  //
//Upon invocation we hand off control by spawning a new task to actually perform    //
//the reset of the system. Other services could be acted upon with more logic.      //
//**********************************************************************************//
enum msgCodeEnum multiServiceHandler(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	TW_LOG(TW_TRACE,"multiServiceHandler - Function called");
	if (!content) {
		TW_LOG(TW_ERROR,"multiServiceHandler - NULL content pointer");
		return TWX_BAD_REQUEST;
	}
	if (strcmp(entityName, progSets.tw_name) == 0) {
		if (strcmp(serviceName, "Reset") == 0) {
			/* Create a task to handle the shutdown so we can respond gracefully */
			twApi_CreateTask(1, resetTask);
		}
		return TWX_NOT_FOUND;
	}
	return TWX_NOT_FOUND;
}

//*********************************************************************************//
//                             blinkPin()                                          //
//                                                                                 //
//This function is called whenever a request is received from the server to invoke //
//the blinkPin service. It takes two arguments, the pin to blink and how many times//
//it should blink the pin. A blink is defined on a pin by toggling it's state at   //
//least twice, ON-OFF-ON or LOW-HIGH-LOW.                                          //
//*********************************************************************************//
enum msgCodeEnum blinkPin(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
char *pinName=NULL;
//double pinNumber;
double blinkct=0;
int pin_id=-1;
pid_t child_pid=0;

         TW_LOG(TW_TRACE,"BlinkPinService - Function called");
         if (!params || !content) {
                 TW_LOG(TW_ERROR,"BlinkPinService - NULL params or content pointer");
                 return TWX_BAD_REQUEST;
         }
		 
         twInfoTable_GetString(params, "pinName", 0, &pinName);
         twInfoTable_GetNumber(params, "numberOfBlinks", 0, &blinkct);
		 pin_id=name2Pin_Id(pinName);
		 if(pin_id>0) {
			child_pid=fork();
			if(child_pid!=0) {
				printf("Spawned new process for blinks: %d\n", child_pid);
				*content = twInfoTable_CreateFromNumber("result", blinkct);
				return TWX_SUCCESS;
				}
			else {
			  
			 for(blinkct=blinkct; blinkct>0; blinkct--){
				pin_writeValue(pin_id, PIN_HIGH);
				sleep(1);
				pin_writeValue(pin_id, PIN_LOW);
				sleep(1);
				printf("blink ct: %f\n", blinkct);
				}
				printf("Quitting Child process\n");
				exit(0);
			  }
			}
			else {
				TW_LOG(TW_ERROR,"BlinkPinService - %s is not a known pin", pinName);
				}
         *content = twInfoTable_CreateFromNumber("result", blinkct);
        return TWX_SUCCESS;
        
}

//********************************************************************************//
//                             Data Collection Task()                             //
//                                                                                //
//This function gets called at the rate defined in the task creation.  The SDK has//
// a simple cooperative multitasker, so the function cannot infinitely loop.      //
// Use of a task like this is optional and not required in a multithreaded        //
// environment where this functionality could be provided in a separate thread.   //
//********************************************************************************//
#define DATA_COLLECTION_RATE_MSEC 2000
void dataCollectionTask(DATETIME now, void * params) {
    
	sendPropertyUpdate(NULL);   
    
}


//*****************************************************************************//
//                            ThingWorxTask()                                  //
//                                                                             //
//The start of the ThingWorx magic. We start by initializing the SDK and then  //
//we declare our properties and open a websocket connection to the server.     //
//*****************************************************************************//
void ThingWorxTask() {
	int prog_err=0;

	twLogger_SetLevel(TW_ERROR);
    if(progSets.debug_log==1) {
	    twLogger_SetLevel(TW_TRACE);
	    twLogger_SetIsVerbose(1);
	}
	TW_LOG(TW_FORCE, "Starting up");

    /* Wait until we have an Internet connection */
	
    	twSleepMsec(5);

	/* Initialize the API */
	prog_err = twApi_Initialize(progSets.tw_host, (int16_t)progSets.tw_port, TW_URI, progSets.tw_appKey, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);

	if (prog_err!=0) {
		TW_LOG(TW_ERROR, "Error initializing the API");
		exit(prog_err);
	}

	/* Allow self signed certs */
	twApi_SetSelfSignedOk();

	/*Register our reset service*/
	twApi_RegisterService(TW_THING, progSets.tw_name, "Reset", NULL, NULL, TW_NOTHING, NULL, multiServiceHandler, NULL);
	/* Register our Blink Pin Service */
	twDataShape *blink_pin_ds=twDataShape_Create(twDataShapeEntry_Create("pinName","The Pin to Blink",TW_NUMBER));
	twDataShape_AddEntry(blink_pin_ds, twDataShapeEntry_Create("numberOfBlinks","The number of times to Blink",TW_NUMBER));
	twApi_RegisterService(TW_THING, progSets.tw_name, "BlinkPin", "Blinks pin(pinNumber) a number of times(blinks)", blink_pin_ds, TW_NOTHING, NULL, blinkPin, NULL);

	/* Register our properties */
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "IPAddress", TW_STRING, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "Name", TW_STRING, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "Location", TW_LOCATION, NULL, "ALWAYS", 0, propertyHandler, NULL);
	//Register our properties: GPIO properties
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "Error", TW_STRING, NULL, "ALWAYS", 0, propertyHandler, NULL);
	
	//Register our properties: FAE I2C Sensors	
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "temperature_adt75", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);

	twApi_RegisterProperty(TW_THING, progSets.tw_name, "humidity", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "barometer", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "altitude", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "light", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "accel_x", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "accel_y", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "accel_z", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	
	//Register our properties: FAE Analog ADC Pins	
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "ADC1", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "ADC2", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "ADC3", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "ADC4", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "ADC5", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "ADC6", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "ADC7", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, progSets.tw_name, "ADC8", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);

	//Register our properties: Raspberry Pi specific pins
  	registerLocalPinProperties(progSets.tw_name, propertyHandler);
	
	/* Bind our thing -- Tells the server we're ready to send data with the name specified in the argument*/
	twApi_BindThing(progSets.tw_name);

	/* Connect to server */
	if (!twApi_Connect(CONNECT_TIMEOUT, -1)) {

		/* Register our "Data collection Task" with the tasker */
		printf("Starting data Collection Task...\n");
		twApi_CreateTask(200, dataCollectionTask);

        
	} else {
		printf("ERROR Establishing websocket to %s:%d\r\n", progSets.tw_host, progSets.tw_port);
	}
	//We keep the thread alive so that the Tasker runs and executes our dataCollectionTask on time.
	while(1) {

		twSleepMsec(5);
	
	}
	//When we break from the loop on program termination we clean up.
	printf("Tearing Down Process....\n");
	twApi_UnbindThing(progSets.tw_name);
	twSleepMsec(100);
	twApi_Delete();
	twLogger_Delete();
	exit(0);
}

//*****************************************************************************//
//                            FAE_INIT()                                       //
//Starts the I2C subsystem and initializes the structures for each sensor      //
//*****************************************************************************//

void fae_init(){
	i2c_init(&i2c, "/dev/i2c-1");
printf("I2C init complete\n");
	adt75_init(&s_temp, &i2c, 0x48);
printf("FAE:adt75 init complete\n");
	mpl_init(&s_baro, &i2c, 0x60);
printf("FAE:baro init complete\n");
	sht21_init(&s_temprh, &i2c, 0x40);
printf("FAE:sht21 init complete\n");
	adxl_init(&s_accel);
printf("FAE:adxl init complete\n");
	tsl_init(&s_lux, &i2c, 0x29);
printf("FAE:lux init complete\n");
	adc_init(&s_adc, &i2c, 0x10);
printf("FAE:adc init complete\n");
}

//*****************************************************************************//
//                            MAIN FUNCTION                                    //
//                                                                             //
//The main entry point of the program. It initializes the hardware and loads   //
//the settings files. It then starts the main ThingWorx task to send data.     //
//*****************************************************************************//
int main()
{

	printf("Starting %s v%s", APPLICATION_NAME, APPLICATION_VERSION);

    loadSettingsFile("agent.properties", &progSets);
	if(progSets.FAE_enable==1) 
		{  
		printf("Initializing I2C/FAE....\n");
		fae_init();
		}
	printf("Initializing other pins....\n");
	initBoardPins();
	readPinConf(progSets.pin_config);
	printf("Thing Name: %s\n", progSets.tw_name);
	printf("Thing Server: %s:%d\n", progSets.tw_host, progSets.tw_port);
	printf("Thing AppKey: %s\n", progSets.tw_appKey);
	printf("Scan Rate: %d\n", progSets.scanRate);

    properties.hostName="test";
    properties.IPAddress="0.0.0.0";

	if(progSets.ipLoc==1){
		getLocation(&currentLoc);
		properties.Location.longitude = currentLoc.latf;
		properties.Location.latitude = currentLoc.lngf;
		}
	else {
		properties.Location.longitude = 0.0;
		properties.Location.latitude = 0.0;
	}
  //start the Thingworx task that will send the data
   ThingWorxTask();

  return 0;
  }
