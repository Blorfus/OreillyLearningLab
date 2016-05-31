#include "Settings.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int g_fd_settingsFile;

char *settingsNames[9]={"tw_host", "tw_port", "AppKey", "ScanRate", "tw_name", "FAE_BoardEnable", "tw_secure", "PinConfig", "Debug"};
extern int parse_reportPinConfig(char *pin_lbl, char *configStr);


int loadSettingsFile(char *fileLocation, tw_settings *settings){

 if(strlen(fileLocation)<=0) {
	return -1;   
	}
     
    g_fd_settingsFile = open(fileLocation, O_RDONLY);   
    if(g_fd_settingsFile == -1) {
	perror("Error occurred while opening the settings file:");
	}

    return parseSettings(g_fd_settingsFile, settings);
}

int parseSettings(int fd_settings, tw_settings *settings){
  char *fileContents;
  ssize_t charsRead;
  char *tmp=NULL;

  fileContents=(char *)calloc(RD_BUFF_SZ, sizeof(char));

  charsRead=read(fd_settings, fileContents, RD_BUFF_SZ);
  printf("Chars Read: %d\n", charsRead);
  tmp=strtok(fileContents, "=");
  
  while(tmp!=NULL) {
    char *val = strtok(NULL, "\n");
    set(tmp, val, settings);
    tmp=strtok(NULL, "=" );
    }
print_settings(settings);

return 1;
}

int set(char *name, char *value, tw_settings *settings) {

  if(strcmp(name, settingsNames[0])==0) { //tw_host
	set_str(settings->tw_host, value, 128);
	}
  else if(strcmp(name, settingsNames[1])==0) {  //tw_port
	set_num(&settings->tw_port, value);
	}
  else if(strcmp(name, settingsNames[2])==0) {  //AppKey
	set_str(settings->tw_appKey, value, 40);
    	}
  else if(strcmp(name, settingsNames[3])==0) { //scanRate
	set_num(&settings->scanRate, value);
	}
  else if(strcmp(name, settingsNames[4])==0) { //Name
	set_str(settings->tw_name, value, 100);
	}
  else if(strcmp(name, settingsNames[5])==0) { 
	if(strcmp(value, "true")==0) {
		set_num(&settings->FAE_enable, "1");
		}
	else {
		set_num(&settings->FAE_enable, "0");
		}
	}
  else if(strcmp(name, settingsNames[6])==0) {
	if(strcmp(value, "true")==0) {
		set_num(&settings->tw_secure, "1");
		}
	else {
		set_num(&settings->tw_secure, "0");
		}
	}
  else if(strcmp(name, settingsNames[7])==0) {
	set_str(settings->pin_config, value, 60);
	}
  else if(strcmp(name, settingsNames[8])==0) {
	if(strcmp(value, "true")==0) {
		set_num(&settings->debug_log, "1");
		}
	else {
		set_num(&settings->debug_log, "0");
		}
	}
   printf("Set:%s v:%s\n", name, value);
  return 0;
  }

int set_str(char *dest, char *val, int sz){
   return snprintf(dest, sz, "%s", val);	
   }

void set_num(int *dest, char *val) {
   *dest=atoi(val);
   }

void print_settings(tw_settings *settings){

  printf("Settings: tw_host::%s, tw_port::%d, AppKey::%s, ScanRate::%d, Name::%s, tw_secure::%d, FAE_enable::%d, PinConfig::%s, Debug::%d\n", settings->tw_host, settings->tw_port, settings->tw_appKey, settings->scanRate,settings->tw_name,settings->tw_secure,settings->FAE_enable,settings->pin_config,settings->debug_log);

}
