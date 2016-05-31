//file: pinspectre_rPi.h//
#include "pinspectre_hwbase.h"
#include "bcm2835.h"
#include <string.h>
#include <unistd.h>

#define PIN_CT  15

char *GPIO_NAMES[PIN_CT]= {"GPIO_00","GPIO_01","GPIO_04","GPIO_17","GPIO_21","GPIO_22","GPIO_10","GPIO_09","GPIO_11","GPIO_18","GPIO_23","GPIO_24","GPIO_25","GPIO_08","GPIO_07"};
int GPIO_MAP[PIN_CT] = { RPI_GPIO_P1_03, RPI_GPIO_P1_05, RPI_GPIO_P1_07, RPI_GPIO_P1_11, RPI_GPIO_P1_13,RPI_GPIO_P1_15,RPI_GPIO_P1_19,RPI_GPIO_P1_21,RPI_GPIO_P1_23, RPI_GPIO_P1_12,RPI_GPIO_P1_16,RPI_GPIO_P1_18,RPI_GPIO_P1_22,RPI_GPIO_P1_24,RPI_GPIO_P1_26};
int GPIO_STATE[PIN_CT];  

void initBoardPins(){
  //Start a connection to the BCM2835 chip that will give us access to the GPIO ports
    if (geteuid() == 0 && !getenv("FAKEROOTKEY")) //<--we make sure we're root
      {
      printf("Starting GPIO access...");
      int bcm_res= bcm2835_init();
      if(bcm_res) { printf("Success\n"); }
      else { printf("Error\n"); perror("GPIO:"); }
      }
     else { printf("You need to be root to access the GPIO ports\n"); }

     int ctr=0;
     for(ctr=0; ctr<PIN_CT; ctr++){
	pin_direction( GPIO_MAP[ctr], PIN_OUTPUT);	

	} 
  }


propertyList *getTWPropertyUpdates(propertyList *inputProplist){
   if(inputProplist==NULL) return NULL;
   int ctr=0;
   int localTargetPin=-1;
   
   for(ctr=0; ctr<PIN_CT; ctr++){
	localTargetPin=GPIO_MAP[ctr];
	printf("localTargetPin=%d\n", localTargetPin);
	if(GPIO_STATE[ctr]==PIN_INPUT) {
		twApi_AddPropertyToList(inputProplist, GPIO_NAMES[ctr], twPrimitive_CreateFromNumber(pin_readValue(localTargetPin)), 0);
		}
	}

   return inputProplist;
}

propertyList *getTWPinConfigs(propertyList *inputConfigList) {
  if(inputConfigList==NULL) return NULL;
  int ctr=0;
  char tmp_buff[40];
  char tmp_config[6];
  char *configRef=NULL;
  tmp_config[5]='\0';

  for(ctr=0; ctr<PIN_CT; ctr++) {
      
      snprintf(tmp_buff, 40, "%s%s", GPIO_NAMES[ctr], "_Config");
      configRef=getPinConfig(GPIO_NAMES[ctr]);
      strncpy(tmp_config, configRef, 5);
      if(configRef!=NULL){
	printf("getTWPinConfigs(%s): %s\n", GPIO_NAMES[ctr], tmp_config);
	  twApi_AddPropertyToList(inputConfigList, tmp_buff, twPrimitive_CreateFromString(configRef, 1),0);
	  } 
      }


  return inputConfigList;
}

char *getPinConfig(char *pinName) {
   cJSON *pinNode = getPinNode(pinName);
   cJSON *pinConf=cJSON_GetObjectItem(pinNode, "pinConf");

printf("getPinConfig[%s]: %s\n", pinName, cJSON_Print(pinConf));
    return pinConf->valuestring;
  }

void setPinConfig(char *pinName, char *conf){
   cJSON *pinNode = getPinNode(pinName);
   cJSON *pinConf=cJSON_GetObjectItem(pinNode, "pinConf");

   strcpy(pinConf->valuestring, conf);

 }

cJSON *getPinNode(char *pinName){
int ctr=0;
  cJSON *temp_pinConf=NULL;

  cJSON *pinsArray=cJSON_GetObjectItem(configuration, "pins");
  int total=cJSON_GetArraySize(pinsArray);

  for(ctr=0; ctr<total; ctr++) {
    temp_pinConf=cJSON_GetArrayItem(pinsArray, ctr);
    if(strcmp(temp_pinConf->child->string, pinName)) {
	printf("getPinNode: %s\n", cJSON_Print(temp_pinConf->child));
        return temp_pinConf->child;
        }
    }
  return NULL;
}



int configName2Pin_Id(char *configName) {
  char newName[strlen(configName)];
  char *newEnd=NULL;
  int len=0;
 
  newEnd=strrchr(configName, '_');
  len=newEnd-configName;
  strncpy(newName, configName, len);
printf("configName2Pin_id: newName: %s\n", newName);
  return name2Pin_Id(newName);
  }

int pinId2Ordinal(int pin_id) {
int ctr;

        for(ctr=0; ctr<PIN_CT; ctr++) {
                if(GPIO_MAP[ctr]==pin_id) {
                        return ctr;
                        }
          }
   return -1;
}

int name2Pin_Id(char *name){
	int ctr;
	
	for(ctr=0; ctr<PIN_CT; ctr++) {
		if(strcmp(GPIO_NAMES[ctr], name)==0) {
			return GPIO_MAP[ctr];
			}
	  }
   return -1;
  }

char *id_2Name(int pin_id) {
   int ctr;
	
   for(ctr=0; ctr<PIN_CT; ctr++) {
     if(GPIO_MAP[ctr]==pin_id) {
	   return GPIO_NAMES[ctr];
	   }
	}
  return NULL;
}  


void registerLocalPinProperties(char *thingName, void *propertyHandler){
  int ctr=0;
  char configName[40];
  
  for(ctr=0; ctr<PIN_CT; ctr++) {
       printf("Registering Property NAME: %s\n", GPIO_NAMES[ctr]);
       twApi_RegisterProperty(TW_THING, thingName, GPIO_NAMES[ctr], TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	snprintf(configName, 40, "%s_Config", GPIO_NAMES[ctr]);
	printf("Registering Config Property Name: %s\n", configName);
	twApi_RegisterProperty(TW_THING, thingName, configName, TW_STRING, NULL, "ALWAYS", 0, propertyHandler, NULL);
    }

}  

void pin_direction(int pin_id, int direction){
     int ordinal=pinId2Ordinal(pin_id);
	if(direction==PIN_OUTPUT) {
	  GPIO_STATE[ordinal]=PIN_OUTPUT;
	  bcm2835_gpio_set(pin_id);
	  bcm2835_gpio_fsel(pin_id, BCM2835_GPIO_FSEL_OUTP);
	  }
	else {
	  GPIO_STATE[ordinal]=PIN_INPUT;
	  bcm2835_gpio_clr(pin_id);
	  }
}


void pin_writeValue(int pin_id, int value){
printf("-->WriteValue: %d\n", value);
   if(value==1) {
	bcm2835_gpio_write(pin_id, HIGH);
	}
   else if(value!=1) {
	bcm2835_gpio_write(pin_id, LOW);
	}
   else {
	bcm2835_gpio_write(pin_id, value);
	}
   
   }
 
int pin_readValue(int pin_id){
  int reading;
  bcm2835_gpio_fsel(pin_id, BCM2835_GPIO_FSEL_INPT);
  reading=(int)bcm2835_gpio_lev(pin_id);
  printf("Reading pin: %d\n", reading);
  return reading;
}

int pin_readFromName(char *pin_name){
  int pin_id=name2Pin_Id(pin_name);
  return pin_readValue(pin_id);
}

void pin_setType(int pin_id, int type){

//do nothing
}

 

propertyList *go_data(propertyList *inputProplist, int sim){

 
if(sim==1) {

	}
   else {
   
    twApi_AddPropertyToList(inputProplist, "Temperature", twPrimitive_CreateFromNumber(pin_readFromName("GPIO_17")),0);
    twApi_AddPropertyToList(inputProplist, "Pressure", twPrimitive_CreateFromNumber(pin_readFromName("GPIO_21")),0);
	twApi_AddPropertyToList(inputProplist, "In_use", twPrimitive_CreateFromBoolean(pin_readFromName("GPIO_22")),0);
	twApi_AddPropertyToList(inputProplist, "Power_consumption", twPrimitive_CreateFromNumber(pin_readFromName("GPIO_18")),0);

	}
       
  return inputProplist; 
}