#ifndef __PINSPECTRE_H__
#define __PINSPECTRE_H__

#include <stdio.h>
#include "cJSON.h"
#define PIN_HIGH 1
#define PIN_LOW  0


void setPinFromConfStr(char *pin_name, char *pin_conf_str);      //Parses configuration string, commad delimited, I/A/V
void setPin(int pin_id, int input, int digital, int value);	   //Takes raw arguments and sets the pin, convenience wrapper function
int setDirection(int pin_id, int input);				//Sets the direction of the data on the pin, input or output
int setValue(int pin_id, int value);					//Sets the value of the pin if output, enables pullup/pulldown resistor if input
int setValueFromPinName(char *pinName, int value);

int writePinConf(char *fileName, cJSON *settingsOut);		//Writes an array of pinConfigurations
int readPinConf(char *fileName);					//Reads a JSON file with configurations and sets all the pins accordingly
int name2Pin_Id(char *name);

FILE *loadJSONSettingsFile(char *fileName);
int parseJSONSettings(FILE* fd_settings, cJSON *settings);


#endif