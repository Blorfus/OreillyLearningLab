TARGET_PLATFORM=gcc-linux-arm
TW_SDK_ROOT=./src/tw-c-sdk_1_3_1-957
TW_SRC=./src/tw-c-sdk_1_3_1-957/src
TW_LIB=./src/tw-c-sdk_1_3_1-957/build/obj/$(TARGET_PLATFORM)/debug
START_DIR=$(PWD)/src/
SDK_CONFIG = -DENABLE_TASKER=1 -DENABLE_FILE_XFER=0 -DENABLE_TUNNELING=0 -DOFFLINE_MSG_STORE=0

include $(TW_SDK_ROOT)/build/platforms/$(TARGET_PLATFORM)/Makefile.$(TARGET_PLATFORM)

all: app

app:
	gcc src/twc_oll_agent.c src/Settings.c src/pinspectre.c src/pinspectre_rPi.c src/geoloc.c src/cJSON.c -g3 $(CC_OPTS) -I$(TW_SRC)/messaging -I$(TW_SRC)/utils -I$(TW_SRC)/porting -I$(TW_SRC)/websocket -I$(TW_SRC)/tls -I$(TW_SRC)/api/ -I$(TW_SRC)/config/ -I./src -I./src/Headers -L./src -L$(TW_LIB) -w -lm -lTw -lsensor -lcurl -lz -lbcm2835 -o twc_agent

clean:
	rm -f ./src/Headers
	rm -f./twc_agent
