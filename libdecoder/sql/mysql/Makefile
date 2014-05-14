TOP_DIR = ../../..
MODULE_TOP_DIR = .
BUILD_NAME = libmysql_inj
SUB_MOD =
SUB_DIR = 
BUILD_TYPE = dynlib
CFLAGS_LOCAL = 

include $(TOP_DIR)/script/compile.mk

prepare_local:
	$(BISON) mysql.y

clean_local:
	$(RM) mysql mysql_parser.*
