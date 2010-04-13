#if !defined(SVIPC_SEM_H)
#define SVIPC_SEM_H

#ifdef __cplusplus
extern "C" {
#endif

int svipc_sem_init(key_t key, int numslots);
int svipc_sem_cleanup(key_t key);
int svipc_sem_info(key_t key, int details);
int svipc_semtake(key_t key,int id,float wait);
int svipc_semgive(key_t key,int id);

#ifdef __cplusplus
}
#endif

#endif
