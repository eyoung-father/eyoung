TOP_DIR = ../../..
MODULE_TOP_DIR = .
BUILD_NAME = libhtml_decode
SUB_MOD =
SUB_DIR = 
BUILD_TYPE = dynlib
CFLAGS_LOCAL = -Wno-sign-compare -I../include

include $(TOP_DIR)/script/compile.mk

prepare_local:
	$(BISON) html.y
	$(FLEX) html.l
	$(FREPLACE) YYSTYPE HTML_STYPE html_lex.h
	$(FREPLACE) YYSTYPE HTML_STYPE html_lex.c
	$(FREPLACE) html__ html_ html_lex.c
	$(FREPLACE) html__ html_ html_lex.h

clean_local:
	$(RM) html_parser.* html_lex.* html
