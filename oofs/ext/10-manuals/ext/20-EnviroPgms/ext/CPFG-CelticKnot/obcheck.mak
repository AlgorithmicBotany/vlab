LOCAL="."

CC=gcc
LIBS=-L$(LOCAL) -lcomm

obcheck: obcheck.c
	$(CC) -o obcheck obcheck.c message.c $(LIBS)
