#if !defined(SVIPC_H)
#define SVIPC_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************
 * Debug
 *******************************************************************/

extern int svipc_debug;

#define Debug(level, fmt, ...) { \
    if(level<=svipc_debug) { \
       fprintf (stderr,"(%02d) %15s:%-4d - %15s: ",level,__FILE__,__LINE__,__PRETTY_FUNCTION__); \
       fprintf (stderr, fmt, ## __VA_ARGS__); \
       fflush (stderr); \
    } \
 }

typedef enum {
	SVIPC_CHAR,
	SVIPC_SHORT,
	SVIPC_INT,
	SVIPC_LONG,
	SVIPC_FLOAT,
	SVIPC_DOUBLE
} slot_type;

#if defined(SVIPC_SZ_DEF)
int slot_type_sz[] = {
	sizeof(char),
	sizeof(short),
	sizeof(int),
	sizeof(long),
	sizeof(float),
	sizeof(double)
};
#endif

typedef struct {
   int typeid;       // data type
   int countdims;    // number of dimensions
   int *number;      // number elts on each dimension
   void* data;       // data pointer
} slot_array;

long svipc_ftok(char *path, int proj);
int  svipc_shm_info(long key, long details);
int  svipc_shm_init(long key, long numslots);
int  svipc_shm_write(long key, char *id, slot_array *a, int publish);
int  svipc_shm_read(long key, char *id, slot_array *a, int subscribe);
int  svipc_shm_free(long key, char* id);
int  svipc_shm_cleanup(long key);

#ifdef __cplusplus
}
#endif

#endif
