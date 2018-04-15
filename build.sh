#!/usr/bin/env bash

OPEN_SDK_ROOT_DIR=/home/bclouser/workspace/esp-open-sdk
PATH_TO_SDK="${OPEN_SDK_ROOT_DIR}/ESP8266_NONOS_SDK-2.1.0-18-g61248df"
SDK_LIBS_DIR="${ESP_OPEN_SDK_ROOT_DIR}/sdk/lib"
TTY_PORT=/dev/ttyUSB0
ESPTOOL="${OPEN_SDK_ROOT_DIR}/esptool/esptool.py"

#LIBTOOL="${MOUNTED_CASE_SENSITIVE}/esp-open-sdk/crosstool-NG/.build/xtensa-lx106-elf/build/build-gmp-host-x86_64-build_apple-darwin15.5.0/libtool"
LIBTOOL=libtool
BUILD_FLAVOR="release"


#export PATH=/Volumes/case-sensitive/esp-open-sdk/xtensa-lx106-elf/bin:$PATH
export PATH=${OPEN_SDK_ROOT_DIR}/xtensa-lx106-elf/bin:$PATH


## These are all for the mqtt library... need to find a way to abstract this.
export FLAVOR=debug
export VERBOSE=true
export USE_OPENSDK=yes


if [ $# -lt 1 ];then
	echo "Please provide name of project to build. (must match subdirectory)"
	exit -1
fi

cd $1 || {
	echo "Failed to enter project directory. Does it exist?"
	exit -1
} 

# --debug=b
#make VERBOSE=true SDK_BASE=${PATH_TO_SDK} FLAVOR=${BUILD_FLAVOR} ESPPORT=${TTY_PORT} ESPTOOL=${ESPTOOL} LIBTOOL=${LIBTOOL} $1

make VERBOSE=true SDK_BASE=${PATH_TO_SDK} FLAVOR=${BUILD_FLAVOR} SDK_LIB=${SDK_LIBS_DIR} ESPPORT=${TTY_PORT} ESPTOOL=${ESPTOOL} LIBTOOL=${LIBTOOL} $2

