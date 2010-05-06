// System-V IPC plugin wrapper
// Matthieu D.C. Bec 23/04/2010
// GNU Public License (GPLv3) applies - see www.gnu.org

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>

#include "svipc_misc.h"

int svipc_debug = 0;

//---------------------------------------------------------------
// svipc_ftok
//---------------------------------------------------------------
key_t svipc_ftok(char *path, int proj)
{
   key_t key = ftok(path, proj);
   if (key == -1)
      perror("ftok failed");
   return key;
}

//---------------------------------------------------------------
// svipc_nprocs
//---------------------------------------------------------------
long svipc_nprocs(void)
{
   // These values may not be standard
   // _SC_NPROCESSORS_CONF - number of processors configured
   // _SC_NPROCESSORS_ONLN - number of processors online
   return sysconf(_SC_NPROCESSORS_ONLN);
}

//---------------------------------------------------------------
// hacks
//---------------------------------------------------------------

#if defined(SVIPC_HACKS)
   /* nanosleep */
   #include <time.h>
   /* EAGAIN */
   #include <errno.h>
   int semtimedop (int semid, struct sembuf *sops, size_t nsops, struct timespec *timeout) {
      int status;

      struct timespec sleepquantum;
      sleepquantum.tv_sec = 0;
      sleepquantum.tv_nsec = SVIPC_SLEEPQUANTUM;

      long time_to_live;

      if (timeout!=NULL)
         time_to_live = timeout->tv_sec * 1e9 + timeout->tv_nsec;
      else 
         time_to_live = -1;


      if (time_to_live>=0) {
         sops->sem_flg |= IPC_NOWAIT;
         status = EAGAIN;

         while (time_to_live > 0 ) {
            if ( (status = semop (semid, sops, nsops)) != EAGAIN ) break;
            nanosleep(&sleepquantum,NULL);
            time_to_live -= SVIPC_SLEEPQUANTUM;
         }
      } else {
         status = semop (semid, sops, nsops);
      }

      return status;
   }
#endif
