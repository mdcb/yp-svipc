#if !defined(SVIPC_MISC_H)
#define SVIPC_MISC_H

#ifdef __cplusplus
extern "C" {
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

#ifdef __cplusplus
}
#endif

#endif
