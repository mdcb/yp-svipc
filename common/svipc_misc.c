#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include "svipc_misc.h"

int svipc_debug=0;

//---------------------------------------------------------------
// svipc_ftok
//---------------------------------------------------------------
long svipc_ftok(char *path, int proj) {
   long key = (long) ftok(path,proj);
   if (key == -1) perror ("ftok failed");
   return key;
}

