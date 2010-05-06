// System-V IPC plugin wrapper
// Matthieu D.C. Bec 23/04/2010
// GNU Public License (GPLv3) applies - see www.gnu.org

#if !defined(SVIPC_MSQ_H)
#define SVIPC_MSQ_H

#ifdef __cplusplus
extern "C"
{
#endif

   int svipc_msq_init(key_t key);
   int svipc_msq_cleanup(key_t key);
   int svipc_msq_info(key_t key, int details);
   int svipc_msq_snd(key_t key, long mtype, size_t msgsz, void *msgp, int nowait);
   int svipc_msq_rcv(key_t key, long mtype, void **msgp, int nowait);

#ifdef __cplusplus
}
#endif

#endif
