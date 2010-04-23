// System-V IPC plugin wrapper
// Matthieu D.C. Bec 23/04/2010
// GNU Public License (GPLv3) applies - see www.gnu.org

#if !defined(SVIPC_SHM_H)
#define SVIPC_SHM_H

#ifdef __cplusplus
extern "C"
{
#endif

   typedef enum
   {
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

   typedef struct
   {
      int typeid;               // data type
      int countdims;            // number of dimensions
      int *number;              // number elts on each dimension
      void *data;               // data pointer
   } slot_array;

   int svipc_shm_init(key_t key, int numslots);
   int svipc_shm_cleanup(key_t key);
   int svipc_shm_info(key_t key, int details);
   int svipc_shm_write(key_t key, char *id, slot_array * a, int publish);
   int svipc_shm_read(key_t key, char *id, slot_array * a, float subscribe);
   int svipc_shm_free(key_t key, char *id);

   int release_slot_array(slot_array * a);

#if !defined(SVIPC_NOSEGFUNC)
   int svipc_shm_attach(key_t key, char *id, slot_array * a);
   int svipc_shm_detach(void *addr);
#endif

#ifdef __cplusplus
}
#endif

#endif
