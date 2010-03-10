########################################################################
#			Compiler Flags
########################################################################
#
CFLAGS = -Wall -g -O0 
#-pg

CPPFLAGS = -Wall -g -O0 -DBOOST_ALL_NO_LIB=1 -DBOOST_SYSTEM_DYN_LINK=1
#-pg

########################################################################
#			Project Configure
########################################################################
ROOT_DIR = ..

BOOST_DIR = /home/tengjiao.zhao/code/mywork/boost_1_41_0

VPATH = $(ROOT_DIR)/src/ : $(ROOT_DIR)/src/include : $(ROOT_DIR)/src/common : $(ROOT_DIR)/src/main 

CPP_SRCS = main.cpp

INCS = 	-I$(BOOST_DIR) \
	-I$(ROOT_DIR)/src \
	-I$(ROOT_DIR)/src/include \
	-I$(ROOT_DIR)/src/common \
	-I$(ROOT_DIR)/src/main \

EXE = simple-dns

LIBS = -lpthread

include $(ROOT_DIR)/template.mk