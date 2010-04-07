#if !defined(SVIPC_SHM_H)
#define SVIPC_SHM_H

#ifdef __cplusplus
extern "C" {
#endif

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

int svipc_shm_init(long key, long numslots);
int svipc_shm_cleanup(long key);
int svipc_shm_info(long key, long details);
int svipc_shm_write(long key, char *id, slot_array *a, int publish);
int svipc_shm_read(long key, char *id, slot_array *a, float subscribe);
int svipc_shm_free(long key, char* id);

#if !defined(SVIPC_NOSEGFUNC)
int svipc_shm_attach(long key, char *id, slot_array *a);
int svipc_shm_detach(void *addr);
#endif

#ifdef __cplusplus
}
#endif

#endif