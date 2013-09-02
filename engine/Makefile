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
	$(BISON) gram.y
	$(FLEX) gram.l
	$(FREPLACE) YYSTYPE ey_stype gram_lexer.c
	$(FREPLACE) YYSTYPE ey_stype gram_lexer.h
	$(FREPLACE) YYLTYPE ey_ltype gram_lexer.c
	$(FREPLACE) YYLTYPE ey_ltype gram_lexer.h
	$(FREPLACE) gram__ gram_ gram_lexer.c
	$(FREPLACE) gram__ gram_ gram_lexer.h

clean_local:
	$(RM) gram_lexer.c gram_lexer.h gram_parser.c gram_parser.h gram_parser.output
