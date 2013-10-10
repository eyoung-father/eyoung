TOP_DIR = ..
MODULE_TOP_DIR = .
BUILD_TYPE = dynlib
BUILD_NAME = libjit
SUB_MOD =
SUB_DIR = 
ARCH ?= x86-64
TARGETOS ?= Linux
CFLAGS_LOCAL = -DWITHOUT_LIBTCC -Wno-sign-compare -Wno-missing-field-initializers

SRCS = libjit.c libtcc.c tccpp.c tccgen.c tccelf.c tccasm.c tccrun.c
ifeq ($(ARCH), i386)
	SRCS += i386-gen.c i386-asm.c
endif

ifeq ($(ARCH), x86-64)
	SRCS += x86_64-gen.c i386-asm.c
endif

ifeq ($(ARCH), arm)
	SRCS += arm-gen.c
endif

include $(TOP_DIR)/script/compile.mk

