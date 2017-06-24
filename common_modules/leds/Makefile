THISDIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SRC_DIR:=$(THISDIR)
BUILD_DIR:=$(THISDIR)build
FIRMWARE_DIR:=$(THISDIR)firmware
LIB:=$(FIRMWARE_DIR)/libleds.a

SRC:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)*.c))
OBJ	= $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRC)))

$(BUILD_DIR)/%.o: %.c
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $< -o $@

.PHONY: all

all: createDirs $(OBJ)
	$(AR)  rcs $(LIB) $(OBJ)

createDirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(FIRMWARE_DIR)
