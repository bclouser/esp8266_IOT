#!/usr/bin/env bash

# On a mac, we need a case sensitive volumte for building. I just made one and mounted it
MOUNTED_CASE_SENSITIVE="/Volumes/case-sensitive"
PATH_TO_SDK="${MOUNTED_CASE_SENSITIVE}/v2.0.0_16_07_19/ESP8266_NONOS_SDK"
TTY_PORT=/dev/tty.SLAB_USBtoUART
ESPTOOL="${MOUNTED_CASE_SENSITIVE}/esp-open-sdk/esptool/esptool.py"
#LIBTOOL="${MOUNTED_CASE_SENSITIVE}/esp-open-sdk/crosstool-NG/.build/xtensa-lx106-elf/build/build-gmp-host-x86_64-build_apple-darwin15.5.0/libtool"
LIBTOOL=libtool
BUILD_FLAVOR="release"

export PATH=$PATH:${MOUNTED_CASE_SENSITIVE}/esp-open-sdk/xtensa-lx106-elf/bin

cd shadeControl

# --debug=b
make SDK_BASE=${PATH_TO_SDK} FLAVOR=${BUILD_FLAVOR} ESPPORT=${TTY_PORT} ESPTOOL=${ESPTOOL} LIBTOOL=${LIBTOOL} $1
