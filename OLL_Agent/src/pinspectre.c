#include <string.h>
#include "pinspectre.h"
#include "pinspectre_hwbase.h"
#include "cJSON.h"

#define RD_BUFF_SZ  4096

void setPinFromConfStr(char *pin_name, char *pin_conf_str)      //Parses configuration string, commad delimited, I/A/V
{
  char holder[20];
  char *temp;
  char *type;

   int pin_id=-1;
   snprintf(holder, 20, "%s", pin_conf_str);

   if(strstr(pin_name, "_Config")==NULL) {
      pin_id=name2Pin_Id(pin_name);
      }
   else {
      pin_id=configName2Pin_Id(pin_name);
     }
printf("tokenizing for %s[%d]....%s\n", pin_name, pin_id, pin_conf_str);  
  temp=strtok(holder, ",");

  if(temp!=NULL) {
     if(strcmp("O", temp)==0) { setDirection(pin_id, PIN_OUTPUT);  }
     else if(strcmp("I", temp)==0) { setDirection(pin_id, PIN_INPUT);  }
     }

  temp=strtok(NULL, ",");
  type=temp;

  if(temp!=NULL)  {
     if(strcmp("A", temp)==0) { pin_setType(pin_id, PIN_ANALOG);  }
     else if(strcmp("D", temp)==0) { pin_setType(pin_id, PIN_DIGITAL);  }
     }

  temp=strtok(NULL, ",");

  if(temp!=NULL) {
	if(strcmp("D", type)==0) {
   	  printf("DigitalType\n");
	    if(strcmp(temp, "1")==0) {
	        setValue(pin_id, PIN_HIGH);
		}
	    else {
	      setValue(pin_id, PIN_LOW);
	      }
	    }
	else {
	    printf("Writing as analog");
	    setValue(pin_id, atoi(temp));
	    }
	}

}

void setPin(int pin_id, int input, int digital, int value)	   //Takes raw arguments and sets the pin, convenience wrapper function
{
  setDirection(pin_id, input);
  pin_setType(pin_id, digital);
  setValue(pin_id, value);
}



int writePinConf(char *fileName, cJSON *settingsOut){		//Writes an array of pinConfigurations to a file
//Open the file
  if(strlen(fileName)<=0) {
        return -1;
        }
    FILE *settingsFile=loadJSONSettingsFile(fileName);
//convert JSON into string and write to file.
	char *outputSettingsTxt=cJSON_Print(settingsOut); 
	fwrite(outputSettingsTxt, 1, strlen(outputSettingsTxt), settingsFile); 

   return 0;
}

int readPinConf(char *fileName)								//Reads a JSON file with configurations and sets all the pins accordingly
  {
  //cJSON *settings=NULL;
  FILE *settingsFile=loadJSONSettingsFile(fileName);
  parseJSONSettings(settingsFile, configuration);//Global defined in the pinspectre_hwbase.h file
  int ctr=0;
  if(configuration==NULL) { printf("ERROR: Pin Config file unparseable\n");  }
  else { printf("Parsing Successful, Pin Configuration loaded\n");  }
 //we parse through the JSON now and load settings  

  cJSON *pinsArray=cJSON_GetObjectItem(configuration, "pins");
  int total=cJSON_GetArraySize(pinsArray);
  cJSON *temp_pinConf=NULL;
  cJSON *pinConf_ele=NULL;

  for(ctr=0; ctr<total; ctr++) {
    temp_pinConf=cJSON_GetArrayItem(pinsArray, ctr); 
    pinConf_ele=cJSON_GetObjectItem(temp_pinConf->child, "pinConf");
    setPinFromConfStr(temp_pinConf->child->string, pinConf_ele->valuestring);
    }
  return 0;
  }
//=========================================[FILE I/O]==================================================================================================
FILE *loadJSONSettingsFile(char *fileName){
  FILE *settingsFile=NULL;
 if(strlen(fileName)<=0) {
        return NULL;
        }
printf("Attempting to load pinConfiguration file: %s\n", fileName);
    settingsFile = fopen(fileName, "r+");
    if(settingsFile == NULL) {
        perror("Error occured while opening the settings file:");
        }
    return settingsFile;
}

int parseJSONSettings(FILE* fd_settings, cJSON *settings){
  char *fileContents;
  ssize_t charsRead=0;
  //char *tmp=NULL;

  fileContents=(char *)calloc(RD_BUFF_SZ, sizeof(char));
  charsRead=fread(fileContents, 1, RD_BUFF_SZ, fd_settings);

  printf("Chars Read: %d\nParsing JSON...", charsRead);
  configuration=(cJSON*)cJSON_Parse(fileContents);
   free(fileContents);
   fclose(fd_settings);
return 1;
}



int setDirection(int pin_id, int input){					   //Sets the direction of the data on the pin, input or output
  pin_direction(pin_id, input);
  return 1;
  }
int setValue(int pin_id, int value){						   //Sets the value of the pin if output, enables pullup/pulldown resistor if input
  pin_writeValue(pin_id, value);
  return 1;
  }

int setValueFromPinName(char *pinName, int value){
  int pin_id=name2Pin_Id(pinName);
  setValue(pin_id, value);
  printf("Setting Pin %s(%d) to %d\n", pinName, pin_id, value);
  return 1;
  }

int setType(int pin_id, int pin_type){
  pin_setType(pin_id, pin_type);
  return 1;
  }
