TOP_DIR = ..
MODULE_TOP_DIR = .
BUILD_TYPE = dynlib
BUILD_NAME = libtcc
SUB_MOD =
SUB_DIR = 
ARCH ?= x86-64
TARGETOS ?= Linux
CFLAGS_LOCAL = -Wno-sign-compare -Wno-missing-field-initializers

SRCS = libtcc.c tccpp.c tccgen.c tccelf.c tccasm.c tccrun.c
ifeq ($(ARCH), i386)
	SRCS += i386-gen.c i386-asm.c
	CFLAGS_LOCAL += -DTCC_TARGET_I386
endif

ifeq ($(ARCH), x86-64)
	SRCS += x86_64-gen.c i386-asm.c
	CFLAGS_LOCAL += -DTCC_TARGET_X86_64
endif

ifeq ($(ARCH), arm)
	SRCS += arm-gen.c
	CFLAGS_LOCAL += -DTCC_TARGET_ARM -DWITHOUT_LIBTCC
endif

include $(TOP_DIR)/script/compile.mk

