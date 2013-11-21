TOP_DIR = ../../..
MODULE_TOP_DIR = .
BUILD_NAME = libhttp
SUB_MOD =
SUB_DIR = 
BUILD_TYPE = dynlib
CFLAGS_LOCAL = -Wno-sign-compare -I../include

include $(TOP_DIR)/script/compile.mk

prepare_local:
	$(BISON) http_client.y
	$(BISON) http_server.y
	$(FLEX) http_client.l
	$(FLEX) http_server.l
	$(FREPLACE) YYSTYPE HTTP_CLIENT_STYPE http_client_lex.h
	$(FREPLACE) YYSTYPE HTTP_CLIENT_STYPE http_client_lex.c
	$(FREPLACE) YYSTYPE HTTP_SERVER_STYPE http_server_lex.h
	$(FREPLACE) YYSTYPE HTTP_SERVER_STYPE http_server_lex.c

clean_local:
	$(RM) http_client_parser.* http_client_lex.* http_server_parser.* http_server_lex.* http
