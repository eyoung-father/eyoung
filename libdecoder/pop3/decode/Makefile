TOP_DIR = ../../..
MODULE_TOP_DIR = .
BUILD_NAME = libpop3
SUB_MOD =
SUB_DIR = 
BUILD_TYPE = dynlib
CFLAGS_LOCAL = -Wno-sign-compare -I../include

include $(TOP_DIR)/script/compile.mk

prepare_local:
	$(BISON) pop3_client.y
	$(BISON) pop3_server.y
	$(FLEX) pop3_client.l
	$(FLEX) pop3_server.l
	$(FREPLACE) YYSTYPE POP3_CLIENT_STYPE pop3_client_lex.h
	$(FREPLACE) YYSTYPE POP3_CLIENT_STYPE pop3_client_lex.c
	$(FREPLACE) YYSTYPE POP3_SERVER_STYPE pop3_server_lex.h
	$(FREPLACE) YYSTYPE POP3_SERVER_STYPE pop3_server_lex.c
	$(FREPLACE) pop3_client__ pop3_client_ pop3_client_lex.c
	$(FREPLACE) pop3_client__ pop3_client_ pop3_client_lex.h
	$(FREPLACE) pop3_server__ pop3_server_ pop3_server_lex.c
	$(FREPLACE) pop3_server__ pop3_server_ pop3_server_lex.h

clean_local:
	$(RM) pop3_client_parser.* pop3_client_lex.* pop3_server_parser.* pop3_server_lex.* pop3
