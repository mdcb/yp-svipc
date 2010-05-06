// System-V IPC plugin wrapper
// Matthieu D.C. Bec 23/04/2010
// GNU Public License (GPLv3) applies - see www.gnu.org

#if !defined(SVIPC_MISC_H)
#define SVIPC_MISC_H


#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************
 * plugin
 *******************************************************************/

   key_t svipc_ftok(char *path, int proj);
   long svipc_nprocs(void);

/*******************************************************************
 * debug
 *******************************************************************/

   extern int svipc_debug;

#define Debug(level, fmt, ...) { \
    if(level<=svipc_debug) { \
       fprintf (stderr,"(%02d) %15s:%-4d - %15s: ",level,__FILE__,__LINE__,__PRETTY_FUNCTION__); \
       fprintf (stderr, fmt, ## __VA_ARGS__); \
       fflush (stderr); \
    } \
 }



/*******************************************************************
 * compat hacks
 *******************************************************************/
 
#if defined(SVIPC_HACKS)
   #include <sys/sem.h>
   #define SVIPC_SLEEPQUANTUM 1000 // quantum of time in nanoseconds to sleep between timedop try
   int semtimedop (int semid, struct sembuf *sops, size_t nsops, struct timespec *timeout);
#endif



#ifdef __cplusplus
}
#endif

#endif
