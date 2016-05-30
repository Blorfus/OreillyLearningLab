
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define RD_BUFF_SZ  2048    //The number of characters that will be read from the settings file at any time.

typedef struct{
  char tw_host[128];
  int  tw_port;
  char tw_appKey[40];
  int  scanRate;
  char tw_name[100];
  int FAE_enable;
  int tw_secure;
 // char serialPort[30];
 // int sim_mode;
  char pin_config[60];
  int debug_log;
}tw_settings;

int loadSettingsFile(char *fileLocation, tw_settings *settings);

int parseSettings(int fd_settings, tw_settings *settings);
int set(char *name, char *value, tw_settings *settings);
int set_str(char *dest, char *val, int sz);
void set_num(int *dest, char *val);
void print_settings(tw_settings *settings);

#endif
