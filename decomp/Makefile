EE_FILE_NAME = decomp
ADDRESS = 000a0000

EE_OBJS_DIR = obj/
EE_BIN_DIR = bin/
EE_ELF = $(EE_BIN_DIR)$(EE_FILE_NAME).elf
EE_BIN = $(EE_BIN_DIR)$(EE_FILE_NAME).bin
EE_OBJS = main.o

EE_DEFS = -DFIRST_DEF
EE_DEFS += -DDUAL_VIPERS
# EE_DEFS += -DMAGMA_CANNON

ifneq (,$(findstring -DDUAL_VIPERS,$(EE_DEFS)))
    EE_OBJS += dualvipers.o
endif
ifneq (,$(findstring -DMAGMA_CANNON,$(EE_DEFS)))
    EE_OBJS += magmacannon.o
endif

$(info Before transform: EE_OBJS = $(EE_OBJS))
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)
$(info After transform: EE_OBJS = $(EE_OBJS))

EE_LIBS = -ldl -lkernel

EE_INCS := \
	-I../../include \
	-I$(PS2SDK)/ports/include

EE_LDFLAGS = \
	-fno-builtin \
	-nostdlib \
	-nostartfiles \
	-L. \
	-L../../lib \
	-L$(PS2SDK)/ports/lib

EE_CFLAGS := \
	-D_PROGRAM_ENTRY=0x$(ADDRESS) \
	$(EE_DEFS)

#
# Debug build
#
ifeq ($(filter debug,$(MAKECMDGOALS)),debug)
    EE_CFLAGS += -DDEBUG
endif

all: linker $(EE_OBJS_DIR) $(EE_BIN_DIR) $(EE_BIN) code

debug: all

$(EE_OBJS_DIR):
	mkdir -p $(EE_OBJS_DIR)

$(EE_BIN_DIR):
	mkdir -p $(EE_BIN_DIR)

$(EE_OBJS_DIR)%.o : %.c
	$(EE_C_COMPILE) -c $< -o $@

$(EE_OBJS_DIR)%.o : %.S
	$(EE_C_COMPILE) -c $< -o $@

linker:
	$(EE_CC) -E -x c $(EE_CFLAGS) ../linkfile.template | grep -v '^#' > linkfile

code:
	bin2code $(EE_BIN) $(ADDRESS) $(EE_BIN).raw raw-writeonce
	bin2code $(EE_BIN) $(ADDRESS) $(EE_BIN).pnach pnach-writeonce
	cat template/footer.raw >> $(EE_BIN).raw
	cat template/footer.pnach >> $(EE_BIN).pnach

clean:
	rm -rf linkfile
	rm -rf $(EE_OBJS_DIR)
	rm -rf $(EE_BIN_DIR)

include $(CURDIR)/../Makefile.pref
include $(CURDIR)/../Makefile.eeglobal