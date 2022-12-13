TEMPLATE = lib
CONFIG  += staticlib
TARGET   = comm
SOURCES  = comm_master.c comm_slave.c communication.c compression.c

MY_BASE  = ../..
MY_LIBS  = 
include( $${MY_BASE}/common.pri )

