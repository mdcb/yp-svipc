// System-V IPC plugin wrapper
// Matthieu D.C. Bec 23/04/2010
// GNU Public License (GPLv3) applies - see www.gnu.org

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "svipc_misc.h"
#include "svipc_msq.h"

//---------------------------------------------------------------
// svipc_msq_init
//---------------------------------------------------------------
int svipc_msq_init(key_t key)
{
   int msgqid;
   
   Debug(5, "svipc_msq_init %x\n", key);

   msgqid = msgget(key, IPC_CREAT | IPC_PRIVATE | IPC_EXCL | 0666);
   if (msgqid == -1) {
      perror("msgget failed");
      return -1;
   }
   
   return 0;
}

//---------------------------------------------------------------
// svipc_msq_cleanup
//---------------------------------------------------------------
int svipc_msq_cleanup(key_t key)
{
   int msgqid;
   
   Debug(5, "svipc_msq_cleanup\n");

   msgqid = msgget(key, 0666);
   if (msgqid == -1) {
      perror("msgget failed");
      return -1;
   }

   int status = msgctl(msgqid, IPC_RMID, NULL);

   if (status == -1) {
      perror("msgctl IPC_RMID failed");
      return -1;
   }

   return 0;
}

//---------------------------------------------------------------
// svipc_msq_info
//---------------------------------------------------------------
int svipc_msq_info(key_t key, int details)
{
   int msgqid, status;

   Debug(5, "svipc_msq_info %x\n", key);

   msgqid = msgget(key, 0666);
   if (msgqid == -1) {
      perror("msgget failed");
      return -1;
   }
   
   struct msqid_ds stat;
   status = msgctl(msgqid, IPC_STAT, &stat);
   if (status == -1) {
      perror("msgctl IPC_STAT failed");
      return -1;
   }

   if (details) {
      fprintf(stderr, "MsgQ msqid: 0x%x id: %d\n", key, msgqid);
      fprintf(stderr, "Last snd time:  %s", ctime(&stat.msg_stime));
      fprintf(stderr, "Last rcv time: %s", ctime(&stat.msg_rtime));
      fprintf(stderr, "Maximum number of bytes allowed in queue: %ld\n", stat.msg_qbytes);
      fprintf(stderr, "PID of last msgsnd: %d\n", stat.msg_lspid);
      fprintf(stderr, "PID of last msgrcv: %d\n", stat.msg_lrpid);
   }

   fprintf(stderr, "Current number of messages in queue: %ld\n", stat.msg_qnum);

   return 0;
}

//---------------------------------------------------------------
// svipc_msq_snd
//---------------------------------------------------------------
int svipc_msq_snd(key_t key, struct svipc_msgbuf *sendmsg, size_t msgsz, int nowait)
{
   int msgqid, status;

   Debug(5, "svipc_msq_snd %x\n", key);

   msgqid = msgget(key, 0666);
   if (msgqid == -1) {
      perror("msgget failed");
      return -1;
   }
   
   // look how big is the queue
   struct msqid_ds stat;
   status = msgctl(msgqid, IPC_STAT, &stat);
   if (status == -1) {
      perror("msgctl IPC_STAT failed");
      return -1;
   }
   
   if (msgsz>stat.msg_qbytes) {
      perror("msg too big for queue!");
      return -1;
   }

   int msgflg = nowait?IPC_NOWAIT:0;
   
   status = msgsnd(msgqid, sendmsg, msgsz, msgflg);
   if (status == -1) {
      perror("msgget failed");
      return -1;
   }
   
   Debug (1,"msgsnd mtype %ld - nbytes %d sent\n", sendmsg->mtype, (int)msgsz);
   
   return 0;
}

//---------------------------------------------------------------
// svipc_msq_rcv
//---------------------------------------------------------------

int svipc_msq_rcv(key_t key, long mtype, struct svipc_msgbuf **rcvmsg, int nowait)
{
   int msgqid;

   Debug(5, "svipc_msq_rcv\n");

   msgqid = msgget(key, 0666);
   if (msgqid == -1) {
      perror("msgget failed");
      return -1;
   }
   
   int msgflg = nowait?IPC_NOWAIT:0;

   // look how big is the queue
   struct msqid_ds stat;
   int status = msgctl(msgqid, IPC_STAT, &stat);
   if (status == -1) {
      perror("msgctl IPC_STAT failed");
      return -1;
   }

   // all rounder - one message can be as big as the queue itself
   *rcvmsg = malloc( sizeof(struct svipc_msgbuf) + stat.msg_qbytes);
   
   ssize_t nbytes = msgrcv(msgqid, *rcvmsg, stat.msg_qbytes, mtype, msgflg);
   if (nbytes == -1) {
      perror("msgrcv failed");
      return -1;
   }
   
   Debug (5, "msgrcv mtype %ld - nbytes %d\n", mtype, (int) nbytes);

   return 0;

}
