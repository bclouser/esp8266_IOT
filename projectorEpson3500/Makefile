

THIS_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

COMMON_MODULES_DIR := $(THIS_DIR)../common_modules

# name for the target project
TARGET		:= projectorControl
# which modules (subdirectories) of the project to include in compiling

SRC_DIR := $(THIS_DIR)

COMMON_MODULES		:= mqtt wifi message serial
COMMON_INC			:= $(COMMON_MODULES_DIR)/mqtt/include \
							$(COMMON_MODULES_DIR)/mqtt/mqtt/include \
							$(COMMON_MODULES_DIR)/wifi \
							$(COMMON_MODULES_DIR)/message \
							$(COMMON_MODULES_DIR)/serial \
							$(COMMON_MODULES_DIR)

SDK_LIBS = c gcc phy pp net80211 wpa main lwip crypto ssl json driver pwm m
SDK_INC = include include/json
# Output directors to store intermediate compiled files
# relative to the project directory
BUILD_DIR			:= $(THIS_DIR)build
FIRMWARE_BASE		:= $(THIS_DIR)firmware
USER_MODULES        := ./


include ../common_modules/Makefile