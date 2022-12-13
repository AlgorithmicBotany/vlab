

#include "warningset.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

char *gettxt(const char *s1, const char *s2) { return ""; }

void Change_Resolution(void) {}

int GetNewClient(int wait) { return -1; }

int SetServer(int port) { return 0; }

FILE *GetClientsData(void) { return NULL; }

void CopyFromPixmap(void) {}

int pixmaps_exist;

void GetWindowOrigin(int *x, int *y) {
  *x = 0;
  *y = 0;
}

void Change_Filename(char *str) {}
