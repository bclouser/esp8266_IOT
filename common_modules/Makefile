
# name for the target project
TARGET ?= 
# which modules (subdirectories) of the project to include in compiling
COMMON_MODULES		?= 
COMMON_INC			?= 
SDK_LIBS ?= c gcc phy pp net80211 wpa main lwip crypto ssl json driver pwm
SDK_INC ?= include
SDK_DRIVER_LIB_INC ?= driver_lib/include
ALL_COMMON_LIBS := wifi leds message ioPins serial
# Output directors to store intermediate compiled files
# relative to the project directory
BUILD_DIR			?= build
FIRMWARE_BASE		?= firmware


# none sdkota espboot rboot
OTA ?= none
OTA_APP_ADDR = 0x2000
OTA_BOOTLOADER_PATH = ../esp-bootloader/firmware/espboot.bin

THISDIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
# Base directory for the compiler. Needs a / at the end; if not set it'll use the tools that are in
# the PATH.
XTENSA_TOOLS_ROOT ?=

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= /tools/esp8266/sdk/ESP8266_NONOS_SDK

#Esptool.py path and port
ESPTOOL		?= /tools/esptool/esptool.py
ESPPORT		?= /dev/tty.SLAB_USBtoUART
#ESPPORT		?= /dev/tty.wchusbserial1410
#ESPDELAY indicates seconds to wait between flashing the two binary images
ESPDELAY	?= 3
#ESPBAUD		?= 115200
ESPBAUD		?= 460800
ESPTOOL_OPTS=--port $(ESPPORT) --baud $(ESPBAUD)

ESP_MQTT_LIB_DIR := $(THISDIR)/mqtt
ESP_MQTT_LIB := $(ESP_MQTT_LIB_DIR)/firmware/libmqtt.a

# 40m 26m 20m 80m
ESP_FREQ = 40m
# qio qout dio dout
ESP_MODE = dio
#4m 2m 8m 16m 32m
ESP_SIZE = 32m

#32m
ESP_INIT_DATA_DEFAULT_ADDR = 0xfc000

ifeq ("$(ESP_SIZE)","16m")
	ESP_INIT_DATA_DEFAULT_ADDR = 0x1fc000
else ifeq ("$(ESP_SIZE)","32m")
	ESP_INIT_DATA_DEFAULT_ADDR = 0x3fc000
endif

SDK_LIBDIR = lib

# Opensdk patches stdint.h when compiled with an internal SDK. If you run into compile problems pertaining to
# redefinition of int types, try setting this to 'yes'.
USE_OPENSDK ?= yes

DATETIME := $(shell date "+%Y-%b-%d_%H:%M:%S_%Z")

# select which tools to use as compiler, librarian and linker
export CC		:= $(XTENSA_TOOLS_ROOT)xtensa-lx106-elf-gcc
export AR		:= $(XTENSA_TOOLS_ROOT)xtensa-lx106-elf-ar
export LD		:= $(XTENSA_TOOLS_ROOT)xtensa-lx106-elf-gcc
export OBJCOPY	:= $(XTENSA_TOOLS_ROOT)xtensa-lx106-elf-objcopy
export LIB_TOOL  := $(LIBTOOL)


####
#### no user configurable options below here
####
COMMON_INC += $(THISDIR)
export INCDIR	:= $(addprefix -I,$(SRC_DIR))
export EXTRA_INCDIR	:= $(addprefix -I,$(COMMON_INC))
export MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_LIBS 		:= $(addprefix -l,$(SDK_LIBS))

export SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INC)) $(addprefix -I$(SDK_BASE)/,$(SDK_DRIVER_LIB_INC))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
ASMSRC		= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.S))

OBJ		= $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRC)))
OBJ		+= $(patsubst %.S,$(BUILD_DIR)/%.o,$(notdir $(ASMSRC)))

COMMON_LIBS     := $(foreach module,$(COMMON_MODULES), $(THISDIR)$(module)/firmware/lib$(module).a)
JOINED_LIBRARY := $(BUILD_DIR)/joined_library.a
TARGET_LIB := $(BUILD_DIR)/appBase.a
COMBINE_LIBS_SCRIPT := $(BUILD_DIR)/combineLibs.mri
TARGET_OUT	:= $(addprefix $(BUILD_DIR)/,$(TARGET).out)

# compiler flags using during compilation of source files
# CFLAGS		= -g			\
# 						-Wpointer-arith		\
# 						-Wundef			\
# 						-Wl,-EL			\
# 						-Wno-implicit-function-declaration \
# 						-fno-inline-functions	\
# 						-nostdlib       \
# 						-mlongcalls	\
# 						-mno-text-section-literals \
# 						-ffunction-sections \
# 						-fdata-sections	\
# 						-fno-builtin-printf\
# 						-DICACHE_FLASH \
# 						-DBUID_TIME=\"$(DATETIME)\"

CFLAGS		= -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start

LD_SCRIPT	= -T$(SDK_BASE)/ld/eagle.app.v6.ld

OUTPUT := $(addprefix $(FIRMWARE_BASE)/,$(TARGET))

ESPTOOL_WRITE = write_flash --flash_freq $(ESP_FREQ) --flash_mode $(ESP_MODE) --flash_size $(ESP_SIZE) \
									0x00000 $(OUTPUT)0x00000.bin \
									0x10000 $(OUTPUT)0x10000.bin \
									$(ESP_INIT_DATA_DEFAULT_ADDR) $(SDK_BASE)/bin/esp_init_data_default.bin

vpath %.c $(SRC_DIR)

export CFLAGS
export LDFLAGS

define compile-objects
$1/%.o: %.c
	echo "CC $$<"
	$(info $$EXTRA_INCDIR is [${EXTRA_INCDIR}])
	$(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef

%.o: %.c
	$(info $$EXTRA_INCDIR is [${EXTRA_INCDIR}])
	$(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@


.PHONY: all checkdirs clean

all: banner checkdirs $(OUTPUT)

banner:
	@echo "-------------------------------------------\n"
	@echo "BUILD TIME $(DATETIME)"
	@echo "-------------------------------------------\n"

checkdirs: $(BUILD_DIR) $(FIRMWARE_BASE)

$(OUTPUT): $(TARGET_OUT)
	@echo "FW $@"
	$(ESPTOOL) elf2image $(ESPTOOL_FLASHDEF) $< -o $(OUTPUT)

$(BUILD_DIR):
	mkdir -p $@

$(FIRMWARE_BASE):
	mkdir -p $@

$(TARGET_OUT): $(JOINED_LIBRARY)
	$(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(SDK_LIBS) $(JOINED_LIBRARY) -Wl,--end-group -o $@

$(JOINED_LIBRARY): $(TARGET_LIB) $(COMMON_MODULES)
	# create linker script
	echo "CREATE $(JOINED_LIBRARY)" > $(COMBINE_LIBS_SCRIPT)
	echo "ADDLIB $(TARGET_LIB)" >> $(COMBINE_LIBS_SCRIPT)
	$(foreach lib,$(COMMON_LIBS), echo "ADDLIB $(lib)" >> $(COMBINE_LIBS_SCRIPT);)
	echo "SAVE" >> $(COMBINE_LIBS_SCRIPT)
	echo "END" >> $(COMBINE_LIBS_SCRIPT)
	# Run linker script
	$(AR) -M < $(COMBINE_LIBS_SCRIPT)

$(TARGET_LIB): $(OBJ)
	$(info $$EXTRA_INCDIR is [${EXTRA_INCDIR}])
	# Create the base of the final static lib with the object files from the app
	$(AR) rcu $(TARGET_LIB) $(OBJ)


####### Explicit rules for these independent libraries... lots
mqtt:
	# create static lib for esp_mqtt
	make -C $(ESP_MQTT_LIB_DIR) SDK_BASE=$(SDK_BASE) lib

# All of these are built the same and don't require any special rules
$(ALL_COMMON_LIBS):
	make -C $(THISDIR)$@ SDK_BASE=$(SDK_BASE)


flash:
	$(ESPTOOL) $(ESPTOOL_OPTS) $(ESPTOOL_WRITE)

fast: all flash openport

openport:
	echo "After flash, terminal will enter serial port screen"
	echo "Please exit with command:"
	echo "\033[0;31m" "Ctrl + A + k" "\033[0m"

	#@read -p "Press any key to continue... " -n1 -s
	@screen $(ESPPORT) 115200

clean:
	make -C $(ESP_MQTT_LIB_DIR) clean
	rm -rf $(BUILD_DIR)
	rm -rf $(FIRMWARE_BASE)
	$(foreach module, $(COMMON_MODULES), rm -rf $(THISDIR)$(module)/build)
	$(foreach module, $(COMMON_MODULES), rm -rf $(THISDIR)$(module)/firmware)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))


