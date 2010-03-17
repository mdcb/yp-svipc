#if !defined(SVIPC_SEM_H)
#define SVIPC_SEM_H

#ifdef __cplusplus
extern "C" {
#endif

int svipc_sem_init(long key, long numslots);
int svipc_sem_cleanup(long key);
int svipc_sem_info(long key, long details);
int svipc_semtake(long key,long id,float wait);
int svipc_semgive(long key,long id);

#ifdef __cplusplus
}
#endif

#endif
