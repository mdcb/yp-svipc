// System-V IPC plugin wrapper
// Matthieu D.C. Bec 23/04/2010
// GNU Public License (GPLv3) applies - see www.gnu.org

#if !defined(SVIPC_MSQ_H)
#define SVIPC_MSQ_H

#ifdef __cplusplus
extern "C"
{
#endif

   typedef struct svipc_msgbuf
   {
      long mtype;         /* message type, must be > 0 */
      char mtext[1];      /* pointer to message data   */
   } svipc_msgbuf;

   int svipc_msq_init(key_t key);
   int svipc_msq_cleanup(key_t key);
   int svipc_msq_info(key_t key, int details);
   int svipc_msq_snd(key_t key, struct svipc_msgbuf *sendmsg, size_t msgsz, int nowait);
   int svipc_msq_rcv(key_t key, long mtype, struct svipc_msgbuf **rcvmsg, int nowait);

#ifdef __cplusplus
}
#endif

#endif
