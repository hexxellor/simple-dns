
########################################################################
#			Compiler Flags
########################################################################
CFLAGS = -Wall -g -O0 
#-pg

CPPFLAGS = -Wall -g -O0
#-pg

########################################################################
#			Project Configure
########################################################################
ROOT_DIR = ..

VPATH = $(ROOT_DIR)/include : $(ROOT_DIR)/main

C_SRCS = main.c sllp-socket.c sllp-select.c sllp-socketserver.c echo.c snif.c server.c

CPP_SRCS = 

INCS = -I$(ROOT_DIR)/include -I$(ROOT_DIR)/main

EXE = sllp

LIBS = -lpthread

include $(ROOT_DIR)/template.mk