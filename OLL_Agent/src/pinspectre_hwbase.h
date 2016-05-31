//file: pinspectre_base.h//

#ifndef  __PINSPECTRE_HWBASE_H__
#define __PINSPECTRE_HWBASE_H__

#include "twApi.h"
#include "cJSON.h"

#define PIN_ANALOG   0
#define PIN_DIGITAL  1
#define PIN_INPUT    0
#define PIN_OUTPUT   1

cJSON *configuration;



int configName2Pin_Id(char *configName);
char *id_2Name(int pin_id);
int name2Pin_Id(char *name);

cJSON *getPinNode(char *pinName);
void setPinConfig(char *pinName, char *conf);
char *getPinConfig(char *pinName);

propertyList *getTWPropertyUpdates(propertyList *proplist);
void registerLocalPinProperties(char *thingName, void *propertyHandler);
propertyList *getTWPinConfigs(propertyList *inputConfigList);

propertyList *go_data(propertyList *inputProplist, int sim);

void pin_direction(int pin_id, int direction);
void pin_writeValue(int pin_id, int value);
int pin_readValue(int pin_id);
int pin_readFromName(char *pin_name);
void pin_setType(int pin_id, int type);  //Only for systems with dualmode pins (A/D)

void initBoardPins();

#endif
