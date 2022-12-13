LOCAL="."

CC=gcc
LIBS=-L$(LOCAL) -lcomm

mkbackground: mkbackground.c
	$(CC) -o mkbackground mkbackground.c $(LIBS)
