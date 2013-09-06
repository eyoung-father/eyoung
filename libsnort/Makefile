DEMO ?= 0

TOP_DIR = ..
MODULE_TOP_DIR = .
BUILD_NAME = libsnort
SUB_MOD =
SUB_DIR = 
CFLAGS_LOCAL = -DSNORT_DEBUG=1 -Wno-sign-compare
ifeq ($(DEMO),1)
BUILD_TYPE = bin
BUILD_NAME = converter
CFLAGS_LOCAL += -DSNORT_DEBUG=1 -DSNORT_MAIN=1 -Wno-sign-compare
LDFLAGS_LOCAL = -lmemory -ldl
else
BUILD_TYPE = dynlib
endif

include $(TOP_DIR)/script/compile.mk

prepare_local:
	$(BISON) snort.y
	$(FLEX) snort.l
	$(FREPLACE) YYSTYPE SNORT_STYPE snort_lexer.c
	$(FREPLACE) YYSTYPE SNORT_STYPE snort_lexer.h
	$(FREPLACE) YYLTYPE SNORT_LTYPE snort_lexer.c
	$(FREPLACE) YYLTYPE SNORT_LTYPE snort_lexer.h
	$(FREPLACE) gram__ gram_ snort_lexer.c
	$(FREPLACE) gram__ gram_ snort_lexer.h

clean_local:
	rm -rf libsnort.so converter snort_lexer.c snort_lexer.h snort_parser.c snort_parser.h snort_parser.output
