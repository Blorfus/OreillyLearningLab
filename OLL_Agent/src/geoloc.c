
#include "geoloc.h"

static size_t curl_receive_data(char *ptr, size_t size, size_t count, char *stream) {
	int position = 0;
	for(position=0; position<RECEIVE_BUFFER_SIZE; position++) {
		if(stream[position]==0) {
			break;
		}			
	}
	
	int x = 0;
	for(x=0; x<size*count; x++) {
		if(position < RECEIVE_BUFFER_SIZE) {
			stream[position++] = ptr[x];  
		} else {
			break;
		}
	}
	return x;
}

int getLocation(location_t* loc) {
	
	loc->isset = 0;

	//Get a location by calling freegeoip's web service.
	
	char buffer[RECEIVE_BUFFER_SIZE];
	memset(&buffer, 0, RECEIVE_BUFFER_SIZE);
	
	CURL* curl;
	CURLcode result;
	
	curl_global_init(CURL_GLOBAL_ALL);
	
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://freegeoip.net/csv/");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_receive_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
		result = curl_easy_perform(curl);
		
		//Now, parse the data
		if(buffer[0] != 0) {
			int success = sscanf(buffer, "%[^,],%*[^,],%[^,],%*[^,],%[^,],%[^,],%*[^,],%*[^,],%[^,],%[^,],%*d", loc->ip, loc->country, loc->state, loc->city, loc->lat, loc->lng);
			if(success == 6) {
				loc->isset = 1;
				
				sscanf(loc->lat, "%f", &loc->latf);
				sscanf(loc->lng, "%f", &loc->lngf);
				//loc->latf = 12.12;
				//loc->lngf = 24.24;
								
				return 1;
			}
		}
	}
	return 0;
}

void printLocation(location_t* loc) {
	printf("%s, %s, %s\n%s, %s\n%s\n", loc->city, loc->state, loc->country, loc->lat, loc->lng, loc->ip);
}
