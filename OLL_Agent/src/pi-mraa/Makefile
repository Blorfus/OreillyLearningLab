TARGET_PLATFORM=gcc-linux-arm
TW_SDK_ROOT=./src/tw-c-sdk_1_3_1-957
TW_SRC=./src/tw-c-sdk_1_3_1-957/src
TW_LIB=./src/tw-c-sdk_1_3_1-957/build/obj/$(TARGET_PLATFORM)/debug
START_DIR=$(PWD)/src/

include $(TW_SDK_ROOT)/build/platforms/$(TARGET_PLATFORM)/Makefile.$(TARGET_PLATFORM)

all: clean lib app

app:
	gcc src/twc_oll_agent.c src/Settings.c src/geoloc.c src/pinspectre.c src/pinspectre_rPi.c src/cJSON.c -g3 $(CC_OPTS) -I$(TW_SRC)/messaging -I$(TW_SRC)/utils -I$(TW_SRC)/porting -I$(TW_SRC)/api/ -I$(TW_SRC)/config/ -I./src -I./src/Headers -L./src -L$(TW_LIB) -lm -lTw -lsensor -o twc_agent
lib:
	cd $(TW_SDK_ROOT)/build; make PLATFORM=$(TARGET_PLATFORM)
	cd $(TW_LIB); cp libTw.a $(START_DIR)
	mkdir $(START_DIR)/Headers
	cd $(TW_SRC)/api/; cp *.h $(START_DIR)/Headers/
	cd $(TW_SRC)/config/; cp *.h $(START_DIR)/Headers/
	cd $(TW_SRC)/fileTransfer/; cp *.h $(START_DIR)/Headers/
	cd $(TW_SRC)/messaging/; cp *.h $(START_DIR)/Headers/
	cd $(TW_SRC)/porting/; cp *.h $(START_DIR)/Headers/
	cd $(TW_SRC)/tls/; cp *.h $(START_DIR)/Headers/
	cd $(TW_SRC)/utils/; cp *.h $(START_DIR)/Headers/
	cd $(TW_SRC)/websocket/; cp *.h $(START_DIR)/Headers/

clean:
	rm ./src/libTw.a
	rm -Rf ./src/Headers
	rm ./src/twc_agent
