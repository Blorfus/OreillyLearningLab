
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#define RECEIVE_BUFFER_SIZE 512

typedef struct location {
	char isset;
	char ip[50];
	char city[50];
	char state[50];
	char country[50];
	char lat[20];
	char lng[20];
	float latf;
	float lngf;
} location_t;

static size_t curl_receive_data(char *ptr, size_t size, size_t count, char *stream);
int getLocation(location_t* loc);
void printLocation(location_t* loc);