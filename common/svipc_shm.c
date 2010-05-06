// System-V IPC plugin wrapper
// Matthieu D.C. Bec 23/04/2010
// GNU Public License (GPLv3) applies - see www.gnu.org

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* Linux - semtimedop */
#if !defined(__USE_GNU)
#define __USE_GNU
#endif

/* FreeBSD/Darwin - undef semun */
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE
#endif

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "svipc_misc.h"
#define SVIPC_SZ_DEF
#include "svipc_shm.h"

/*******************************************************************
 * define
 *******************************************************************/

#define SLOT_DESC_STRING_MAX 80

union semun
{
   int val;                     /* Value for SETVAL */
   struct semid_ds *buf;        /* Buffer for IPC_STAT, IPC_SET */
   unsigned short *array;       /* Array for GETALL, SETALL */
   struct seminfo *__buf;       /* Buffer for IPC_INFO (Linux specific) */
};

typedef struct
{
   int typeid;
   int countdims;
   int flexible;
} slot_segmap;

typedef struct
{
   int sss_shm_id;
   int sss_sem_id;
   int sss_lock_id;
   int sss_handsake_id;
   slot_segmap *segmap;
} slot_snapshot;

typedef struct
{
   int slot_shmid;
   char desc[SLOT_DESC_STRING_MAX];
} slot_entry;

typedef struct
{
   int master_shmid;
   int master_semid;
   int numslots;
   slot_entry sse[];
} slot_master;

// manages locally attached segments with a linked list
typedef struct _segm
{
   struct _segm *next;
   char id[SLOT_DESC_STRING_MAX];
   void *addr;                  // the seqgment location
   void *pdata;                 // the pointer to data
} _segm;

// attached segment for this session
// fixme: should be an array of segtable[key]
_segm *segtable = NULL;

//---------------------------------------------------------------
// static internals
//---------------------------------------------------------------
/* static int find_master(key_t key); */
static slot_master *attach_master(key_t key);
static int detach_master(slot_master * m);
static int lock_master(slot_master * m);
static int unlock_master(slot_master * m);

static int lkup_slot(slot_master * m, char *id);
static int getfree_slot(slot_master * m);
static int lock_slot(slot_master * m, int slot);
static int unlock_slot(slot_master * m, int slot);
static int free_slot(slot_master * m, int slot);

static void snap_slot(slot_master * m, int slot, slot_snapshot * sss);
static int lock_snaphot(slot_snapshot * sss);
static int unlock_snaphot(slot_snapshot * sss);
static int publish_snapshot(slot_snapshot * sss);
static int subscribe_snapshot(slot_snapshot * sss, struct timespec *pto);

static int acquire_master(key_t key, slot_master ** pm);
static int release_master(slot_master * m);
static int acquire_slot(key_t key, char *id, long *payload, slot_snapshot * sss, struct timespec *pto);
static int release_snapshot(slot_snapshot * sss);

// yorick shm_var/unvar
#if !defined(SVIPC_NOSEGFUNC)
static _segm *seg_add(_segm * list, _segm * item);
static _segm *seg_rem(_segm * list, _segm * item);
static _segm *seg_lkupid(_segm * list, char *id);
static _segm *seg_lkupdata(_segm * list, void *pdata);
#endif

//---------------------------------------------------------------
// private
//---------------------------------------------------------------

/* find_master is an equivalent of 'ipcs' - we dont need it for our
 * implementation and SHM_STAT/SHM_INFO isn't portable across *nix
 * platforms.
 * In the future, we will follow the XSI Interprocess Communication API
 * which is open standard. (man 3p on Linux)
 * static int find_master(key_t key)
 * {
 *    // find the first shm segment associated with a given key
 *    // this only works for key != 0
 * 
 *    struct shm_info info;
 *    struct shmid_ds ds;
 *    int shmid;
 * 
 *    // returns maxid btw
 *    shmid = shmctl(0, SHM_INFO, (struct shmid_ds *) &info);
 *    if (shmid == -1) {
 *       perror("find_master SHM_INFO failed");
 *       return -1;
 *    }
 * 
 *    int i;
 *    for (i = 0; i < info.used_ids; i++) {
 *       shmid = shmctl(i, SHM_STAT, &ds);
 *       Debug(2, "** shmid %d key %d\n", shmid, ds.shm_perm.__key);
 *       if (shmid == -1) {
 *          // no read permission? (on fedora for example, user gdm creates segments with perm=600)
 *          // if we cant read it, it's obviously not the one we are looking for. silently move on next
 *          // perror ("SHM_STAT");
 *          continue;
 *       }
 *       if (shmid != -1 && key == ds.shm_perm.__key) {
 *          return shmid;
 *       }
 *    }
 * 
 *    return -1;
 * }
 */

static slot_master *attach_master(key_t key)
{

   Debug(2, "attach_master %x\n", key);

   int master_shmid = shmget(key, 0, 0666);
   
   if (master_shmid == -1) {
      return NULL;
   }
   return (slot_master *) shmat(master_shmid, NULL, 0);
}

static int detach_master(slot_master * m)
{

   Debug(2, "detach_master\n");

   if (shmdt((void *) m) == -1) {
      perror("detach_master failed");
      return -1;
   }
   return 0;
}

static int lock_master(slot_master * m)
{

   Debug(2, "lock_master\n");

   // lock the master
   struct sembuf sops;
   sops.sem_num = 0;
   sops.sem_op = -1;
   sops.sem_flg = 0;

   int status = semop(m->master_semid, &sops, 1);
   if (status == -1) {
      perror("semop failed");
      return -1;
   }
   return 0;
}

static int unlock_master(slot_master * m)
{

   Debug(2, "unlock_master\n");

   // lock the master
   struct sembuf sops;
   sops.sem_num = 0;
   sops.sem_op = 1;
   sops.sem_flg = 0;

   int status = semop(m->master_semid, &sops, 1);
   if (status == -1) {
      perror("semop failed");
      return -1;
   }
   return 0;
}

static int lkup_slot(slot_master * m, char *id)
{

   Debug(2, "lkup_slot %s\n", id);

   int i;

   for (i = 0; i < m->numslots; i++) {
      if (!strncmp(m->sse[i].desc, id, SLOT_DESC_STRING_MAX))
         return i;
   }
   return -1;
}

static int getfree_slot(slot_master * m)
{

   Debug(2, "getfree_slot\n");

   int i;

   for (i = 0; i < m->numslots; i++) {
      if (m->sse[i].slot_shmid == 0)
         return i;
   }
   return -1;
}

static void snap_slot(slot_master * m, int slot, slot_snapshot * sss)
{
   sss->sss_shm_id = m->sse[slot].slot_shmid;
   sss->sss_sem_id = m->master_semid;
   sss->sss_lock_id = 1 + slot;
   sss->sss_handsake_id = 1 + m->numslots + slot;
}

static int lock_slot(slot_master * m, int slot)
{

   Debug(2, "lock_slot slot %d # %d\n", m->master_semid, slot + 1);

   // lock the slot
   struct sembuf sops;
   sops.sem_num = slot + 1;
   sops.sem_op = -1;
   sops.sem_flg = 0;

   int status = semop(m->master_semid, &sops, 1);
   if (status == -1) {
      perror("semop failed");
      return -1;
   }
   return 0;
}

static int lock_snaphot(slot_snapshot * sss)
{

   Debug(2, "lock_snaphot slot %d # %d\n", sss->sss_sem_id, sss->sss_lock_id);

   // lock the slot
   struct sembuf sops;
   sops.sem_num = sss->sss_lock_id;
   sops.sem_op = -1;
   sops.sem_flg = 0;

   int status = semop(sss->sss_sem_id, &sops, 1);
   if (status == -1) {
      perror("semop failed");
      return -1;
   }
   return 0;
}

static int subscribe_snapshot(slot_snapshot * sss, struct timespec *pto)
{

   Debug(2, "subscribe slot %d # %d\n", sss->sss_sem_id, sss->sss_handsake_id);

   // lock the slot
   struct sembuf sops;
   sops.sem_num = sss->sss_handsake_id;
   sops.sem_op = -1;            // 
   sops.sem_flg = 0;            // fixme - undo if interrupted?

   int status;

   if (pto->tv_sec < 0) {
      // block till update now
      status = semop(sss->sss_sem_id, &sops, 1);
      if (status == -1) {
         perror("semop failed");
         return -1;
      }
   } else {
      // block till update or timeout
      status = semtimedop(sss->sss_sem_id, &sops, 1, pto);
      if (status == -1) {
         if (errno != EAGAIN)
            perror("semop failed");
         return -1;
      }
   }

   return 0;
}

static int publish_snapshot(slot_snapshot * sss)
{

   Debug(2, "publish slot  %d # %d\n", sss->sss_sem_id, sss->sss_handsake_id);

   // how many processes are waiting?
   int numb = semctl(sss->sss_sem_id, sss->sss_handsake_id, GETNCNT);

   // unlock the slot
   struct sembuf sops;
   sops.sem_num = sss->sss_handsake_id;
   sops.sem_op = numb;          // set count to be number of waiters
   sops.sem_flg = 0;

   // wake up
   int status = semop(sss->sss_sem_id, &sops, 1);
   if (status == -1) {
      perror("semop failed");
      return -1;
   }

   sops.sem_num = sss->sss_handsake_id;
   sops.sem_op = 0;             // wait ack from all
   sops.sem_flg = 0;

   // wait till all ack'd
   status = semop(sss->sss_sem_id, &sops, 1);   // fixme - timedop so we don't wait forever + set 0 if that happened
   if (status == -1) {
      perror("semop failed");
      return -1;
   }

   return 0;
}

static int unlock_slot(slot_master * m, int slot)
{

   Debug(2, "unlock_slot slot %d # %d\n", m->master_semid, slot + 1);

   // unlock the slot
   struct sembuf sops;
   sops.sem_num = slot + 1;
   sops.sem_op = 1;
   sops.sem_flg = 0;

   int status = semop(m->master_semid, &sops, 1);
   if (status == -1) {
      perror("semop failed");
      return -1;
   }
   return 0;
}

static int unlock_snaphot(slot_snapshot * sss)
{

   Debug(2, "unlock_snaphot slot %d # %d\n", sss->sss_sem_id, sss->sss_lock_id);

   // lock the slot
   struct sembuf sops;
   sops.sem_num = sss->sss_lock_id;
   sops.sem_op = 1;
   sops.sem_flg = 0;

   int status = semop(sss->sss_sem_id, &sops, 1);
   if (status == -1) {
      perror("unlock_snaphot semop failed");
      return -1;
   }
   return 0;
}

static int free_slot(slot_master * m, int slot)
{

   Debug(2, "free_slot%d\n", slot);

   if (m->sse[slot].slot_shmid != 0) {
      lock_slot(m, slot);

      // free segment
      int status = shmctl(m->sse[slot].slot_shmid, IPC_RMID, 0);
      if (status == -1)
         perror("IPC_RMID failed");

      strcpy(m->sse[slot].desc, "");
      m->sse[slot].slot_shmid = 0;

      unlock_slot(m, slot);
   }

   return 0;
}

//---------------------------------------------------------------
// private api
//---------------------------------------------------------------

static int acquire_master(key_t key, slot_master ** pm)
{
   // look up master
   *pm = attach_master(key);
   if (!*pm) {
      return -1;
   }
   // lock master
   if (lock_master(*pm)) {
      detach_master(*pm);
      return -1;
   }
   return 0;
}

static int release_master(slot_master * m)
{
   int status = 0;

   // release master
   status |= unlock_master(m);
   status |= detach_master(m);
   return status;
}

static int acquire_slot(key_t key, char *id, long *payload, slot_snapshot * sss, struct timespec *pto)
{

   int slot;
   int new = 0;
   slot_master *m;

   // acquire master
   if (acquire_master(key, &m)) {
      Debug(0, "master access error\n");
      return -1;
   }
   // lookup slot
   if ((slot = lkup_slot(m, id)) < 0) {
      if (payload) {
         // not found, payload is set so create one
         slot = getfree_slot(m);
         if (slot < 0) {
            Debug(0, "no slot left\n");
            release_master(m);
            return -1;
         }
         new = 1;
      } else {
         release_master(m);
         Debug(0, "slot not found\n");
         return -1;
      }
   }

   if (pto) {

      // take a snapshot
      snap_slot(m, slot, sss);

      // release master
      release_master(m);

      // blocking -
      int status = subscribe_snapshot(sss, pto);
      if (status) {
         Debug(2, "subscribe failed or timeout\n");
         return -1;
      }

      // lock slot
      if (lock_snaphot(sss)) {
         Debug(0, "failed to acquire lock on slot\n");
         return -1;
      }

   } else {

      // lock slot
      if (lock_slot(m, slot)) {
         Debug(0, "failed to acquire lock on slot\n");
         release_master(m);
         return -1;
      }
      // if new, create a segment
      if (new) {
         int shmid = shmget(key + slot + 1, *payload, 0666 | IPC_CREAT | IPC_EXCL);
         if (shmid == -1) {
            perror("shmget failed");
            unlock_slot(m, slot);
            release_master(m);
            return -1;
         }
         // update the Id/slot name info
         snprintf(m->sse[slot].desc, SLOT_DESC_STRING_MAX, id);
         m->sse[slot].slot_shmid = shmid;

      }
      // take a snapshot
      snap_slot(m, slot, sss);

      // release master
      release_master(m);
   }

   // attach segment
   void *addr = (void *) shmat(sss->sss_shm_id, NULL, 0);
   if (addr == (void *) -1) {
      perror("shmat failed");
      unlock_snaphot(sss);
      return -1;
   }
   // update segmap
   sss->segmap = (slot_segmap *) addr;

   if (new) {
      sss->segmap->typeid = -1;
   }
   // return slot
   return slot;
}

static int release_snapshot(slot_snapshot * sss)
{
   int status;

   // detach segment
   status = shmdt((void *) sss->segmap);
   if (status == -1) {
      perror("shmdt failed");
      unlock_snaphot(sss);
      return -1;
   }
   // unlock slot
   unlock_snaphot(sss);

   return 0;
}

#if !defined(SVIPC_NOSEGFUNC)
//---------------------------------------------------------------
// local segm add/rem/lkup
//---------------------------------------------------------------

static _segm *seg_add(_segm * list, _segm * item)
{
   if (list == NULL) {
      list = item;
      item->next = NULL;
   } else {
      // go to the end of the list
      _segm *cursor = list;
      while (cursor->next != NULL) {
         cursor = cursor->next;
      }
      // and append
      cursor->next = item;
      item->next = NULL;
   }
   return list;
}

static _segm *seg_rem(_segm * list, _segm * item)
{
   _segm *cursor = list;
   _segm *prev = NULL;
   while (cursor != item && cursor->next != NULL) {
      prev = cursor;
      cursor = cursor->next;
   }
   if (prev == NULL) {
      // remove first _segm
      return cursor->next;
   } else if (cursor->next == NULL) {
      return list;
   } else {
      prev->next = cursor->next;
   }
   return list;
}

static _segm *seg_lkupid(_segm * list, char *id)
{
   _segm *cursor = list;
   while (cursor && strcmp(cursor->id, id)) {
      cursor = cursor->next;
   }
   return cursor;
}

static _segm *seg_lkupdata(_segm * list, void *pdata)
{
   _segm *cursor = list;
   while (cursor && cursor->pdata != pdata) {
      cursor = cursor->next;
   }
   return cursor;
}

#endif

//---------------------------------------------------------------
// svipc_shm_info
//---------------------------------------------------------------
int svipc_shm_info(key_t key, int details)
{
   int i;

   slot_master *m;

   // acquire master
   if (acquire_master(key, &m)) {
      Debug(0, "master access error\n");
      return -1;
   }

   fprintf(stderr, "slot   used?   id");
   if (details)
      fprintf(stderr, "     type    dims\n");
   else
      fprintf(stderr, "\n");
   fprintf(stderr, "----------------------------------\n");
   for (i = 0; i < m->numslots; i++) {
      fprintf(stderr, "[%d]   %2d       \"%s\"", i, m->sse[i].slot_shmid != 0, m->sse[i].desc);
      if (details && m->sse[i].slot_shmid != 0) {
         lock_slot(m, i);
         void *addr = (void *) shmat(m->sse[i].slot_shmid, NULL, 0);
         if (addr == (void *) -1)
            perror("shmat failed");
         int typeid = ((int *) addr)[0];
         if (typeid == SVIPC_CHAR)
            fprintf(stderr, "   char ");
         else if (typeid == SVIPC_SHORT)
            fprintf(stderr, "   short ");
         else if (typeid == SVIPC_INT)
            fprintf(stderr, "   int ");
         else if (typeid == SVIPC_LONG)
            fprintf(stderr, "   long ");
         else if (typeid == SVIPC_FLOAT)
            fprintf(stderr, "   float ");
         else if (typeid == SVIPC_DOUBLE)
            fprintf(stderr, "   double ");
         else
            fprintf(stderr, "   indef");

         int countdims = ((int *) addr)[1];
         long totalnumber = 1;
         int *p_addr = (int *) addr + 2;
         for (; countdims > 0; countdims--) {
            fprintf(stderr, ",%d", *p_addr);
            totalnumber *= *p_addr;
            p_addr++;
         }
         fprintf(stderr, "\n");
         shmdt(addr);
         unlock_slot(m, i);
      } else
         fprintf(stderr, "\n");
   }

   release_master(m);

   return 0;

}

//---------------------------------------------------------------
// svipc_shm_init
//---------------------------------------------------------------
int svipc_shm_init(key_t key, int numslots)
{
   // initialize a toplevel pool of semaphores protected memory segments
   // will allow all the process to query/access/etc shared memory

   // a slot is understood as a region in shared memory
   // there will be one semaphore per slot used to synchronise access
   // there will be one semaphore the control access to the master slot
   // the master slot will hold:
   //   it's own semId
   //   <slots> shmid

   int i;

   if (numslots >= 0) {
      // master
      int status;
      int master_shmid;
      int master_semid;

      master_semid = semget(key, 2 * numslots + 1, IPC_CREAT | IPC_PRIVATE | IPC_EXCL | 0666);
      if (master_semid == -1) {
         perror("master_semid semget failed");
         return -1;
      }
      // all locking semaphores are free at startup

      union semun semctlops;
      semctlops.val = 1;
      // fixme - SETALL perf improvement
      for (i = 0; i < numslots + 1; i++) {
         status = semctl(master_semid, i, SETVAL, semctlops);
         if (status == -1) {
            perror("locking semctl failed");
            return -1;
         }
      }

      // all handshake semaphores are empty at startup

      semctlops.val = 0;
      // fixme - SETALL perf improvement
      for (i = 0; i < numslots; i++) {
         status = semctl(master_semid, i + 1 + numslots, SETVAL, semctlops);
         if (status == -1) {
            perror("handshake semctl failed");
            return -1;
         }
      }

      // fixme: there might be race condition at startup when we create the master pool
      // and have not yet to set the master semaphore.

      // create a shm mem pool

      long bytes = sizeof(master_shmid)
         + sizeof(master_semid)
         + sizeof(numslots)
         + numslots * sizeof(slot_entry);

      master_shmid = shmget(key, bytes, 0666 | IPC_CREAT | IPC_EXCL);

      slot_master *m = (slot_master *) shmat(master_shmid, NULL, 0);

      if (m == (slot_master *) - 1) {
         perror("shmat failed");
         return -1;
      }
      memset(m, 0, bytes);

      m->master_shmid = master_shmid;
      m->master_semid = master_semid;
      m->numslots = numslots;
      //flexible arrays
      //m->se=&(m->se);

      for (i = 0; i < numslots; i++) {
         m->sse[i].slot_shmid = 0;
         strcpy(m->sse[i].desc, "");
      }

      // fixme - call something like unlock_master+detach_master

      status = shmdt((void *) m);
      if (status == -1) {
         perror("shmdt failed");
         return -1;
      }

      return 0;

   } else {
      // slave -> noop, print info
      return svipc_shm_info(key, 1);
   }
}

//---------------------------------------------------------------
// svipc_shm_write
//---------------------------------------------------------------
int svipc_shm_write(key_t key, char *id, slot_array * a, int publish)
{
   int status = 0;
   slot_snapshot sss;
   int *p_addr;

   int i;
   int typeid = a->typeid;
   int countdims = a->countdims;
   long totalnumber = 1;
   for (i = 0; i < countdims; i++)
      totalnumber *= *(a->number + i);
   long payload_bytes = totalnumber * slot_type_sz[typeid];     // data
   long shmbytes = 2 * sizeof(int)      // typeID + number of dimensions
      + countdims * sizeof(long)        // size of each dimension
      + payload_bytes;

   if (acquire_slot(key, id, &shmbytes, &sss, NULL) < 0) {
      Debug(0, "acquire_slot failure\n");
      return -1;
   }

   if (sss.segmap->typeid == -1) {
      Debug(2, "new segment, fill headers\n");
      // new segment, fill up header with type, dims and size information
      sss.segmap->typeid = typeid;
      sss.segmap->countdims = countdims;
      p_addr = &sss.segmap->flexible;
      for (i = 0; i < countdims; i++) {
         *p_addr++ = *(a->number + i);
      }
   } else {
      Debug(2, "exisiting segment, check consistency\n");
      // check the reference we have been given is compatible with the one
      // we have in shared memory.
      status = 0;

      if (a->typeid != sss.segmap->typeid) {
         perror("incompatible type");
         status |= 0x1;
      }
      if (a->countdims != sss.segmap->countdims) {
         perror("incompatible dims");
         status |= 0x2;
      }

      p_addr = &sss.segmap->flexible;
      long shm_totalnumber = 1;
      for (i = 0; i < sss.segmap->countdims; i++) {
         shm_totalnumber *= *p_addr++;
      }

      if (totalnumber != shm_totalnumber) {
         perror("incompatible size");
         status |= 0x4;
      }

      if (status) {
         unlock_snaphot(&sss);
         return -1;
      }
   }

   // copy data content
   memcpy((void *) p_addr, a->data, payload_bytes);

   status = release_snapshot(&sss);

   // wake up
   if (publish)
      status |= publish_snapshot(&sss);

   return status;
}

//---------------------------------------------------------------
// svipc_shm_read
//---------------------------------------------------------------
int svipc_shm_read(key_t key, char *id, slot_array * a, float subscribe_t)
{

   slot_snapshot sss;
   int status;

   struct timespec timeout, *pto = NULL;

   if (subscribe_t != 0.0) {
      timeout.tv_sec = (time_t) subscribe_t;
      timeout.tv_nsec = (long int) ((subscribe_t - timeout.tv_sec) * 1e9);
      pto = &timeout;
   }

   if (acquire_slot(key, id, NULL, &sss, pto) < 0) {
      Debug(1, "acquire_slot failure\n");       // debug 1, could be a timeout
      return -1;
   }

   a->typeid = sss.segmap->typeid;
   a->countdims = sss.segmap->countdims;
   int *p_addr = &sss.segmap->flexible;

   int i;
   long totalnumber = 1;

   if (a->number == NULL) {
      a->number = (int *) malloc(a->countdims * sizeof(*a->number));
   }

   for (i = 0; i < a->countdims; i++) {
      a->number[i] = *p_addr++;
      totalnumber *= a->number[i];
   }

   long payload_bytes = totalnumber * slot_type_sz[a->typeid];  // data

   if (a->data == NULL) {
      a->data = malloc(payload_bytes);
   }

   memcpy(a->data, p_addr, payload_bytes);

   status = release_snapshot(&sss);

   return status;
}

//---------------------------------------------------------------
// svipc_shm_free
//---------------------------------------------------------------
int svipc_shm_free(key_t key, char *id)
{

   slot_master *m;

   // acquire master
   if (acquire_master(key, &m)) {
      Debug(0, "master access error\n");
      return -1;
   }

   int slot;
   if ((slot = lkup_slot(m, id)) < 0) {
      Debug(0, "slot not found\n");
      release_master(m);
      return -1;
   }

   free_slot(m, slot);

   release_master(m);

   return 0;

}

//---------------------------------------------------------------
// svipc_shm_cleanup
//---------------------------------------------------------------
int svipc_shm_cleanup(key_t key)
{
   int status;

   slot_master *m;

   // acquire master
   if (acquire_master(key, &m)) {
      Debug(0, "master access error\n");
      return -1;
   }

   int i;

   for (i = 0; i < m->numslots; i++) {
      free_slot(m, i);
   }

   status = semctl(m->master_semid, IPC_RMID, 0);
   if (status == -1) {
      perror("locking semctl IPC_RMID failed");
      return -1;
   }

   status = shmctl(m->master_shmid, IPC_RMID, 0);
   if (status == -1) {
      perror("shmctl IPC_RMID failed");
      return -1;
   }

   detach_master(m);

   return 0;

}

//---------------------------------------------------------------
// cleanup_slot_array
//---------------------------------------------------------------
int release_slot_array(slot_array * a)
{
   free(a->number);
   free(a->data);
   return 0;
}

#if !defined(SVIPC_NOSEGFUNC)
int svipc_shm_attach(key_t key, char *id, slot_array * a)
{
   _segm *this;
   slot_snapshot sss;
   int status = 0;
   slot_segmap *pseg;
   int cleanup = 0;

   if ((this = seg_lkupid(segtable, id)) != NULL) {
      // already refd, return the address.
      pseg = (slot_segmap *) this->addr;
   } else {
      cleanup = 1;
      if (acquire_slot(key, id, NULL, &sss, NULL) < 0) {
         Debug(0, "acquire_slot failure\n");
         return -1;
      }
      // the slot segment is now attached
      // append it to the local lkup
      this = (_segm *) malloc(sizeof(_segm));
      snprintf(this->id, SLOT_DESC_STRING_MAX, id);
      this->addr = sss.segmap;
      segtable = seg_add(segtable, this);
      pseg = sss.segmap;
   }

   a->typeid = pseg->typeid;
   a->countdims = pseg->countdims;
   int *p_addr = &pseg->flexible;
   int i;
   a->number = (int *) malloc(a->countdims * sizeof(*a->number));
   for (i = 0; i < a->countdims; i++) {
      a->number[i] = *p_addr++;
   }
   a->data = p_addr;
   // reverse lookup for unvar use the address of the data - make note of it now
   this->pdata = p_addr;

   if (cleanup) {
      // unlock (but don't detach) slot
      unlock_snaphot(&sss);
   }

   return status;

}

int svipc_shm_detach(void *addr)
{
   int status = 0;
   _segm *this = seg_lkupdata(segtable, addr);
   if (this == NULL) {
      Debug(0, "no attached mem\n");
      return -1;
   } else {
      // remove from lkup table and detach
      segtable = seg_rem(segtable, this);
      Debug(2, "detattach %p\n", this->addr);
      status = shmdt((void *) this->addr);
      strcpy(this->id, "");
      this->addr = NULL;
      this->pdata = NULL;
      if (status == -1)
         perror("shmdt failed");
      return status;
   }
}
#endif
