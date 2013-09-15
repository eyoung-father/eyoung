DEMO ?= 0

TOP_DIR = ..
MODULE_TOP_DIR = .
BUILD_NAME = libengine
SUB_MOD =
SUB_DIR = 
CFLAGS_LOCAL = -DGRAM_LTYPE_IS_DECLARED -DENGINE_DEBUG=1 -Wno-sign-compare
ifeq ($(DEMO),1)
BUILD_TYPE = bin
BUILD_NAME = engine
CFLAGS_LOCAL += -DENGINE_DEBUG=1 -DENGINE_MAIN=1 -Wno-sign-compare
LDFLAGS_LOCAL = -lmemory -ldl -ljit -ltcc
else
BUILD_TYPE = dynlib
endif

include $(TOP_DIR)/script/compile.mk

prepare_local:
	$(BISON) gram.y
	$(FLEX) gram.l
	$(FREPLACE) YYSTYPE GRAM_STYPE gram_lexer.c
	$(FREPLACE) YYSTYPE GRAM_STYPE gram_lexer.h
	$(FREPLACE) YYLTYPE GRAM_LTYPE gram_lexer.c
	$(FREPLACE) YYLTYPE GRAM_LTYPE gram_lexer.h
	$(FREPLACE) gram__ gram_ gram_lexer.c
	$(FREPLACE) gram__ gram_ gram_lexer.h

clean_local:
	$(RM) gram_lexer.c gram_lexer.h gram_parser.c gram_parser.h gram_parser.output
