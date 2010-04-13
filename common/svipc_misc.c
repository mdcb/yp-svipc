#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>

#include "svipc_misc.h"

int svipc_debug=0;

//---------------------------------------------------------------
// svipc_ftok
//---------------------------------------------------------------
key_t svipc_ftok(char *path, int proj) {
   key_t key = ftok(path,proj);
   if (key == -1) perror ("ftok failed");
   return key;
}

//---------------------------------------------------------------
// svipc_nprocs
//---------------------------------------------------------------
long svipc_nprocs(void) {
   // These values may not be standard
   // _SC_NPROCESSORS_CONF - number of processors configured
   // _SC_NPROCESSORS_ONLN - number of processors online
   return sysconf(_SC_NPROCESSORS_ONLN);
}

