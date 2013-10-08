#
# compile tool
#
OPTIMIZE ?= 0
CC = gcc
CPP = gcc
AR = ar
LD = ld
STRIP = strip
FLEX = flex
BISON = bison
RM = rm -rf
MK_FILE = $(TOP_DIR)/script/compile.mk
FREPLACE = $(TOP_DIR)/script/freplace


#
# compile object dir
#
OBJ_DIR = obj

#
# default global compile flags
#
CFLAGS_GLOBAL = -I/usr/include -I$(TOP_DIR)/include
CFLAGS_GLOBAL += -fno-strict-aliasing 		\
				 -Wno-pointer-sign 			\
				 -Wno-unused-parameter 		\
				 -Wno-pointer-to-int-cast 	\
				 -Wno-write-strings 		\
				 -Wno-address
CFLAGS_GLOBAL += -fPIC -g -W -Wall -Werror
ifeq ($(OPTIMIZE), 1)
CFLAGS_GLOBAL += -O2 -DRELEASE=1
endif

CPPFLAGS_GLOBAL = -I/usr/include -I$(TOP_DIR)/include
CPPFLAGS_GLOBAL += -fno-strict-aliasing 	\
				   -Wno-unused-parameter 	\
				   -Wno-pointer-to-int-cast \
				   -Wno-write-strings 		\
				   -Wno-address
CPPFLAGS_GLOBAL += -fPIC -g -W -Wall -Werror
ifeq ($(OPTIMIZE), 1)
CPPFLAGS_GLOBAL += -O2
endif

LDFLAGS_GLOBAL = -L$(TOP_DIR)/build/lib/ -L$(TOP_DIR)/build/prebuild -L/usr/lib 

LIB_DIR = $(TOP_DIR)/build/lib/
BIN_DIR = $(TOP_DIR)/build/bin/

#
# source, object
#
ifndef SRCS
SRCS = $(wildcard *.c)
endif

ifndef CPPSRCS
CPPSRCS = $(wildcard *.cc)
endif

OBJS = $(SRCS:%.c=%.o)
OBJS := $(OBJS:%=$(OBJ_DIR)/%)

CPPOBJS = $(CPPSRCS:%.cc=%.oo)
CPPOBJS := $(CPPOBJS:%=$(OBJ_DIR)/%)

SUB_MOD_OBJS = $(SUB_MOD:%=$(OBJ_DIR)/_%.o)

#
# target file name
#
BUILD_TYPE := $(strip $(BUILD_TYPE))
ifeq ($(BUILD_TYPE), obj)
TARGET = _$(BUILD_NAME).o
endif

ifeq ($(BUILD_TYPE), lib)
TARGET = $(BUILD_NAME).a
endif

ifeq ($(BUILD_TYPE), dynlib)
TARGET = $(BUILD_NAME).so
endif

ifeq ($(BUILD_TYPE), bin)
TARGET = $(BUILD_NAME)
endif

TARGET_FILENAME := $(TARGET)
TARGET := $(OBJ_DIR)/$(TARGET)

#
# install path
#
ifndef INSTALL_DIR
INSTALL_DIR = $(OBJ_DIR)

ifeq ($(BUILD_TYPE), bin)
INSTALL_DIR = $(BIN_DIR)
endif

ifeq ($(BUILD_TYPE), dynlib)
INSTALL_DIR = $(LIB_DIR)
endif

ifeq ($(BUILD_TYPE), lib)
INSTALL_DIR = $(LIB_DIR)
endif

ifeq ($(BUILD_TYPE), obj)
ifeq ($(strip $(TOP_DIR)), .)
${error Invalid build type in top dir}
endif
INSTALL_DIR = ../$(OBJ_DIR)
endif
endif

#
# compile local flag
#
CFLAGS_LOCAL += $(INC_DIR)
CPPFLAGS_LOCAL += $(INC_DIR)

#
# link local flag
#
ifeq ($(BUILD_TYPE), bin)
LDFLAGS_GLOBAL += -lrt -lpthread
endif

#
# default compile script
#
COMPILE = $(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) -MD -c -o $@ $<
COMPILE_CPP = $(CPP) $(CPPFLAGS_GLOBAL) $(CPPFLAGS_LOCAL) -MD -c -o $@ $<

#
# compile process
#
all: $(OBJ_DIR) make_sub $(TARGET) copy_target

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c
	$(COMPILE)

$(OBJ_DIR)/%.oo: %.cc
	$(COMPILE_CPP)

make_sub:
	@for sub in $(SUB_DIR) ; do make -C $$sub ; done

copy_target:
	@if [ ! -d $(INSTALL_DIR) ]	; then /bin/rm -rf $(INSTALL_DIR) ; mkdir $(INSTALL_DIR) ; fi
	/bin/cp -rf -p $(TARGET) $(INSTALL_DIR)

ifeq ($(BUILD_TYPE), obj)
$(TARGET): $(OBJS) $(CPPOBJS) $(SUB_MOD_OBJS)
	$(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) -nostdlib -r -o $(TARGET) $(OBJS) $(CPPOBJS) $(SUB_MOD_OBJS)
endif

ifeq ($(BUILD_TYPE), bin)
$(TARGET): $(OBJS) $(DEP_LIBS_LIST) $(CPPOBJS) $(SUB_MOD_OBJS)
	$(CC) $(OBJS) $(CPPOBJS) $(SUB_MOD_OBJS) $(LDFLAGS_GLOBAL) $(LDFLAGS_LOCAL) -o $@
endif

ifeq ($(BUILD_TYPE), lib)
$(TARGET): $(OBJS) $(CPPOBJS) $(SUB_MOD_OBJS)
	$(AR) rc $(TARGET) $(OBJS) $(CPPOBJS) $(SUB_MOD_OBJS)
endif

ifeq ($(BUILD_TYPE), dynlib)
$(TARGET): $(OBJS) $(CPPOBJS) $(SUB_MOD_OBJS)
	$(CC) -shared -o $(TARGET) $(OBJS) $(CPPOBJS) $(SUB_MOD_OBJS)
endif

ifeq ($(BUILD_TYPE), obj)
endif

#
# prepare process
#
prepare: prepare_local
	@for sub in $(SUB_DIR) ; do make -C $$sub prepare ; done

prepare_local:

#
# strip process
#
strip:
	@for sub in $(SUB_DIR) ; do make -C $$sub strip ; done
ifeq ($(BUILD_TYPE), dynlib)
	$(STRIP) $(TARGET)
endif
ifeq ($(BUILD_TYPE), bin)
	$(STRIP) $(TARGET)
endif

#
# clean process
#
clean: clean_local
	@rm -rf $(OBJ_DIR)
	@for sub in $(SUB_DIR) ; do make -C $$sub clean; done

clean_local:
