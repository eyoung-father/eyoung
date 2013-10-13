TOP_DIR = ../..
MODULE_TOP_DIR = .
BUILD_NAME = pop3
SUB_MOD =
SUB_DIR = 
BUILD_TYPE = obj
CFLAGS_LOCAL = -Wno-sign-compare

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

clean_local:
	$(RM) pop3_client_parser.* pop3_client_lex.* pop3_server_parser.* pop3_server_lex.* pop3
