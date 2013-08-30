DEMO ?= 0

TOP_DIR = ..
MODULE_TOP_DIR = .
BUILD_NAME = libengine
SUB_MOD =
SUB_DIR = 
CFLAGS_LOCAL = -DENGINE_DEBUG=1 -Wno-sign-compare
ifeq ($(DEMO),1)
BUILD_TYPE = bin
BUILD_NAME = engine
CFLAGS_LOCAL += -DENGINE_DEBUG=1 -DENGINE_MAIN=1 -Wno-sign-compare
LDFLAGS_LOCAL = -lmemory -ldl
else
BUILD_TYPE = dynlib
endif

include $(TOP_DIR)/script/compile.mk

prepare_local:
	$(BISON) parse-gram.y
	$(FLEX) scan-gram.l

clean_local:
	$(RM) scan-gram.c scan-gram.h parse-gram.c parse-gram.h 
