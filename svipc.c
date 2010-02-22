#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define __USE_GNU
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "ydata.h"

#define SLOT_DESC_STRING_MAX 80

/*******************************************************************
 * Debug
 *******************************************************************/

int yorick_svipc_debug = 0;

#define Debug(level, fmt, ...) { \
    if(level<=yorick_svipc_debug) { \
       fprintf (stderr,"(%02d) %15s:%-4d - %15s: ",level,__FILE__,__LINE__,__PRETTY_FUNCTION__); \
       fprintf (stderr, fmt, ## __VA_ARGS__); \
       fflush (stderr); \
    } \
 }


union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux specific) */
};

typedef struct {
   int shmid;
   char desc[SLOT_DESC_STRING_MAX];
} slot_entry;

typedef struct {
   int master_shmid;
   int master_semid;
   long numslots;
   slot_entry sse[];
} slot_master;

// manages locally attached segments with a linked list
typedef struct _segm {
      struct _segm* next;
      char id[SLOT_DESC_STRING_MAX];
      void* yaddr;
      void* addr;
} _segm;

// attached segment for this session
// fixme: should be an array of segtable[key]
_segm *segtable = NULL;

//---------------------------------------------------------------
// static protypes
//---------------------------------------------------------------
static int find_master(long key);
static slot_master* attach_master(long key);
static int detach_master(slot_master* m);
static int lock_master(slot_master* m);
static int unlock_master(slot_master* m);
static int lkup_slot(slot_master* m, char *id);
static int getfree_slot(slot_master* m);
static int lock_slot(slot_master* m, int slot);
static int unlock_slot(slot_master* m, int slot);
static int free_slot(slot_master* m, int slot);

static _segm* seg_add(_segm* list, _segm* item);
static _segm* seg_rem(_segm* list, _segm* item);
static _segm* seg_lkupid(_segm* list, char* id);
static _segm* seg_lkupaddr(_segm* list, void* addr);

//---------------------------------------------------------------
// private
//---------------------------------------------------------------

static int find_master(long key) {
   // find the first shm segment associated with a given key
   // this only works for key != 0
   
   struct shm_info info;
   struct shmid_ds ds;
   int shmid;
   
   shmid = shmctl(0, SHM_INFO, (struct shmid_ds *) &info);
   if (shmid == -1) {
      perror ("find_master SHM_INFO failed");
      return -1;
   }
   
   int i;
   for (i=0;i<info.used_ids;i++) {
      shmid = shmctl(i, SHM_STAT, &ds);
      Debug(2, "** shmid %d key %d\n",shmid,ds.shm_perm.__key);
      if (shmid == -1) perror ("SHM_STAT");
      if (shmid != -1 && key == ds.shm_perm.__key) {
         return shmid;
      }
   }
   
   return -1;
}

static slot_master* attach_master(long key) {

   Debug(2, "attach_master %x\n",key);

   int master_shmid=find_master(key);
   
   if (master_shmid==-1) {
      return NULL;
   }
   return (slot_master*) shmat(master_shmid, NULL, 0);
}

static int detach_master(slot_master* m) {

   Debug(2, "detach_master\n");

   if (shmdt((void*)m) == -1) {
      perror ("detach_master failed");
      return -1;
   }
   return 0;
}

static int lock_master(slot_master* m) {

   Debug(2, "lock_master\n");

   // lock the master
   struct sembuf sops;
   sops.sem_num=0;
   sops.sem_op=-1;
   sops.sem_flg=0;

   int status = semop(m->master_semid,&sops,1);
   if (status == -1) {
      perror ("semop failed");
      return -1;
   }
   return 0;
}

static int unlock_master(slot_master* m) {

   Debug(2, "unlock_master\n");

   // lock the master
   struct sembuf sops;
   sops.sem_num=0;
   sops.sem_op=1;
   sops.sem_flg=0;

   int status = semop(m->master_semid,&sops,1);
   if (status == -1) {
      perror ("semop failed");
      return -1;
   }
   return 0;
}

static int lkup_slot(slot_master* m, char *id) {

   Debug(2, "lkup_slot %s\n", id);

   int i;
   
   for (i=0;i<m->numslots;i++) {
      if (!strncmp(m->sse[i].desc,id,SLOT_DESC_STRING_MAX)) return i;
   }
   return -1;
}

static int getfree_slot(slot_master* m) {

   Debug(2, "getfree_slot\n");

   int i;
   
   for (i=0;i<m->numslots;i++) {
      if (m->sse[i].shmid==0) return i;
   }
   return -1;
}

static int lock_slot(slot_master* m, int slot) {
   
   Debug(2, "locking slot %d\n",slot);

   // lock the slot
   struct sembuf sops;
   sops.sem_num=slot+1;
   sops.sem_op=-1;
   sops.sem_flg=0;

   int status = semop(m->master_semid,&sops,1);
   if (status == -1) {
      perror ("semop failed");
      return -1;
   }
   return 0;
}

static int unlock_slot(slot_master* m, int slot) {
   
   Debug(2, "unlock_slot slot %d\n",slot);
   
   // lock the slot
   struct sembuf sops;
   sops.sem_num=slot+1;
   sops.sem_op=1;
   sops.sem_flg=0;

   int status = semop(m->master_semid,&sops,1);
   if (status == -1) {
      perror ("semop failed");
      return -1;
   }
   return 0;
}

static int free_slot(slot_master* m, int slot) {
   
   Debug(2, "free_slot%d\n",slot);
   
   if (m->sse[slot].shmid!=0) {
      lock_slot(m,slot);

      // free segment
      int status = shmctl(m->sse[slot].shmid, IPC_RMID, 0);
      if (status == -1) perror ("IPC_RMID failed");

      snprintf(m->sse[slot].desc,SLOT_DESC_STRING_MAX,"");
      m->sse[slot].shmid = 0;

      unlock_slot(m,slot);
   }

   return 0;
}

//---------------------------------------------------------------
// local segm add/rem/lkup
//---------------------------------------------------------------

static _segm* seg_add(_segm * list, _segm *item) {
   if (list==NULL) {
      list=item;
      item->next = NULL;
   }
   else {
      // go to the end of the list
      _segm *cursor = list;
      while ( cursor->next != NULL ) {
         cursor = cursor->next;
      }
      // and append
      cursor->next = item;
      item->next = NULL;
   }
   return list;
}

static _segm* seg_rem(_segm* list, _segm* item) {
   _segm* cursor = list;
   _segm* prev = NULL;
   while ( cursor != item && cursor->next != NULL ) {
      prev = cursor;
      cursor = cursor->next;
   }
   if (prev==NULL) {
      // remove first _segm
      return cursor->next;
   } else if (cursor->next == NULL) {
      return list;
   } else {
      prev->next=cursor->next;
   }
   return list;
}

static _segm* seg_lkupid(_segm* list, char* id) {
   _segm *cursor = list;
   while ( cursor && strcmp(cursor->id,id) ) {
      cursor = cursor->next;
   }
   return cursor;
}

static _segm* seg_lkupaddr(_segm* list, void* addr) {
   _segm *cursor = list;
   while ( cursor && cursor->yaddr!=addr ) {
      cursor = cursor->next;
   }
   return cursor;
}

//---------------------------------------------------------------
// ftok
//---------------------------------------------------------------
long Y_ftok(char *path, int proj) {
   long key = (long) ftok(path,proj);
   if (key == -1) perror ("ftok failed");
   return key;
   }

//---------------------------------------------------------------
// Y_shm_info
//---------------------------------------------------------------
void Y_shm_info(long key, long details) {
   int i;
  
   // find the master_shmid based on provided key
   // we postulate there is only one segment.
   slot_master *m = attach_master(key);
   
   if (!m) {
      PushIntValue(-1);
      return;
   }
   
   lock_master(m);

   fprintf (stderr, "slot   used?   id");
   if (details)  fprintf (stderr, "     type    dims\n");
   else fprintf (stderr, "\n");
   fprintf (stderr, "----------------------------------\n");
   for (i=0;i<m->numslots;i++) {
      fprintf (stderr, "[%d]   %2d       \"%s\"",i,m->sse[i].shmid!=0,m->sse[i].desc);
      if (details && m->sse[i].shmid!=0) {
         lock_slot(m,i);
         void *addr = (void*)shmat(m->sse[i].shmid, NULL, 0);
         if (addr == (void *) -1) perror ("shmat failed");
         int typeid = ((int*)addr)[0];
         if (typeid==charStruct.dataOps->typeID) fprintf (stderr, "   char ");
         else if (typeid==shortStruct.dataOps->typeID) fprintf (stderr, "   short ");
         else if (typeid==intStruct.dataOps->typeID) fprintf (stderr, "   int ");
         else if (typeid==longStruct.dataOps->typeID) fprintf (stderr, "   long ");
         else if (typeid==floatStruct.dataOps->typeID) fprintf (stderr, "   float ");
         else if (typeid==doubleStruct.dataOps->typeID) fprintf (stderr, "   double ");
         else fprintf (stderr, "   indef");
         
         int countdims = ((int*)addr)[1];
         long totalnumber = 1;
         int *p_addr=(int*)addr+2;
         for(;countdims>0;countdims--) {
            fprintf (stderr, ",%d",*p_addr);
            totalnumber *= *p_addr;
            p_addr++;
         }
         fprintf (stderr, "\n");
         shmdt((void*)m->sse[i].shmid);
         unlock_slot(m,i);
      }
      else fprintf (stderr, "\n");
   }

   unlock_master(m);
   detach_master(m);
      
   PushIntValue(0);
   return;

}

//---------------------------------------------------------------
// Y_shm_init
//---------------------------------------------------------------
void Y_shm_init(long key, long numslots) {
   // initialize a toplevel pool of semaphores protected memory segments
   // will allow all the process to query/access/etc shared memory
   
   // a slot is understood as a region in shared memory
   // there will be one semaphore per slot used to synchronise access
   // there will be one semaphore the control access to the master slot
   // the master slot will hold:
   //   it's own semId
   //   <slots> shmid

   int i;
   
   if (numslots>=0) {
      // master
      int status;
      int master_shmid;
      int master_semid;
      
      master_semid = semget(key,numslots+1,IPC_CREAT|IPC_PRIVATE|IPC_EXCL|0666);
      if (master_semid == -1) {
         perror ("semget failed");
         PushIntValue(-1);
         return;
      }

      // all the semaphores are free at startup
      
      union semun semctlops;
      semctlops.val=1;
      // fixme - SETALL perf improvement
      for (i=0;i<numslots+1;i++) {
         status = semctl(master_semid,i, SETVAL,semctlops);
         if (status == -1) {
            perror ("semctl failed");
            PushIntValue(-1);
            return;
         }
      }

      // fixme: there might be race condition at startup when we create the master pool
      // and have not yet to set the master semaphore.

      // create a shm mem pool
      long bytes = sizeof(master_shmid) + sizeof(master_semid) + sizeof(numslots) + numslots*sizeof(slot_entry);
      master_shmid = shmget(key, bytes, 0666 | IPC_CREAT | IPC_EXCL);

      slot_master *m = (slot_master*) shmat(master_shmid, NULL, 0);
      if (m == (slot_master *)-1) {
         perror ("shmat failed");
         PushIntValue(-1);
         return;
      }
      bzero(m,bytes);

      m->master_shmid=master_shmid;
      m->master_semid=master_semid;
      m->numslots=numslots;
      //m->se=&(m->se);
            
      for (i=0;i<numslots;i++) {
         m->sse[i].shmid=0;
         snprintf(m->sse[i].desc,SLOT_DESC_STRING_MAX,"");
      }
      
      // fixme - call something like unlock_master+detach_master
      
      status = shmdt((void*)m);
      if (status == -1) {
         perror ("shmdt failed");
         PushIntValue(-1);
         }
      
      PushIntValue(0);
      return;
      
   } else {
      // slave -> noop, print info
      Y_shm_info(key,1);
      return;
      }
}

//---------------------------------------------------------------
// Y_shm_write
//---------------------------------------------------------------
void Y_shm_write(long key, char *id, void *a) {
   int slot;
   int new = 0; // flag if writing the first time or attempting to overwrite
   
   // look for the slot in master table
   slot_master *m = attach_master(key);
   if (!m) {
      PushIntValue(-1);
      return;
   }
   
   lock_master(m);
   
   if ( (slot=lkup_slot(m,id)) < 0) {
      // it's a new slot, so create a new one
      slot = getfree_slot(m);
      if (slot<0) {
         Debug(0, "no slot left\n");
         unlock_master(m);
         detach_master(m);
         PushIntValue(-1);
         return;
      }
      new = 1;
   }
   
   if (lock_slot(m,slot))  {
      Debug(0, "failed to acquire lock on slot\n");
      unlock_master(m);
      detach_master(m);
      PushIntValue(-1);
      return;
   }
   
      
   
   int shmid;
   int *p_addr;
   Array *array= (Array *)Pointee(a);
   int typeid = array->type.base->dataOps->typeID;
   int countdims = CountDims(array->type.dims);
   long totalnumber = TotalNumber(array->type.dims); // also as, array->type.number

   long bytes =   2 * sizeof(int)                      // typeID + number of dimensions
                + countdims * sizeof(int)              // size of each dimension
                + totalnumber * array->type.base->size; // data

   //fprintf (stderr, "   element in array %d\n",totalnumber);
   //fprintf (stderr, "   CountDims is %d\n",CountDims(array->type.dims));
   //fprintf (stderr, "   typeID %d\n",array->type.base->dataOps->typeID);
   //fprintf (stderr, "   numbytes %d\n",totalnumber * array->type.base->size);

   if (new) {
      // create a segment
      shmid = shmget((key_t)key+slot+1, bytes, 0666 | IPC_CREAT | IPC_EXCL);
      if (shmid == -1) { 
         perror ("shmget failed");
         unlock_slot(m,slot);
         unlock_master(m);
         detach_master(m);
         PushIntValue(-1);
         return;
      }
   } else {
      shmid = m->sse[slot].shmid;
   }

   // attach segment
   void *addr = (void*)shmat(shmid, NULL, 0);
   if (addr == (void *) -1) { 
      perror ("shmat failed");
      unlock_slot(m,slot);
      unlock_master(m);
      detach_master(m);
      PushIntValue(-1);
      return;
   }
   
   if (new) {
      // fill up the shared mem header with type, dims and size information
      ((int*)addr)[0] = typeid;
      ((int*)addr)[1] = countdims;
      p_addr=(int*)addr+2;
      Dimension *d;
      for (d=array->type.dims;;d=d->next) {
         *p_addr++=d->number;
         if (d->next==NULL) break;
      }
      // update the Id/slot name info
      snprintf(m->sse[slot].desc,SLOT_DESC_STRING_MAX,id);
      m->sse[slot].shmid=shmid;
   } else {
      // check the reference we have been given is compatible with the one
      // we have in shared memory.
      int status = 0;
      int shm_typeid = ((int*)addr)[0];
      int shm_countdims = ((int*)addr)[1];
      long shm_totalnumber = 1;
      p_addr=(int*)addr+2;
      for(;shm_countdims>0;shm_countdims--) {
         shm_totalnumber *= *p_addr++;
      }
      
      if (typeid != ((int*)addr)[0]) {
         perror ("incompatible type");
         status = 0x1;
      }
      if (countdims != ((int*)addr)[1]) {
         perror ("incompatible dims");
         status = 0x2;
      }
      if (totalnumber != shm_totalnumber) {
         perror ("incompatible size");
         status = 0x4;
      }
      if (status) {
         unlock_slot(m,slot);
         unlock_master(m);
         detach_master(m);
         PushIntValue(-1);
         return;
      }
   }
   
   // copy data content
   memcpy((void*)p_addr,a,totalnumber * array->type.base->size);

   int status = shmdt((void*)addr);
   if (status == -1) { 
      perror ("shmdt failed");
      unlock_slot(m,slot);
      unlock_master(m);
      detach_master(m);
      PushIntValue(-1);
      return;
   }

   unlock_slot(m,slot);

   unlock_master(m);
   detach_master(m);
   PushIntValue(0);
   return;
   }

//---------------------------------------------------------------
// Y_shm_read
//---------------------------------------------------------------
void Y_shm_read(long key, char *id) {
   
   slot_master *m = attach_master(key);
   if (!m) {
      PushIntValue(-1);
      return;
   }
      
   lock_master(m);
   
   int slot;
   if ((slot = lkup_slot(m,id)) < 0) {
      Debug(0, "slot not found\n");
      unlock_master(m);
      detach_master(m);
      PushIntValue(-1);
      return;
   }
   
   lock_slot(m,slot);
   
   int shmid = m->sse[slot].shmid;
   
   // attach segment
   void *addr = (void*)shmat(shmid, NULL, 0);
   if (addr == (void *) -1) perror ("shmat failed");



   int typeid = ((int*)addr)[0];
   int countdims = ((int*)addr)[1];
   long totalnumber = 1;

   int *p_addr=(int*)addr+2;

   Dimension *tmp= tmpDims;
   tmpDims= 0;
   FreeDimension(tmp);

   for(;countdims>0;countdims--) {
      //fprintf (stderr, "dim[]=%d\n",*p_addr);
      totalnumber *= *p_addr;
      tmpDims= NewDimension(*p_addr++, 1L, tmpDims);
   }

   Array *a;
   if (typeid==charStruct.dataOps->typeID) a = NewArray(&charStruct, tmpDims);
   else if (typeid==shortStruct.dataOps->typeID) a = NewArray(&shortStruct, tmpDims);
   else if (typeid==intStruct.dataOps->typeID) a = NewArray(&intStruct, tmpDims);
   else if (typeid==longStruct.dataOps->typeID) a = NewArray(&longStruct, tmpDims);
   else if (typeid==floatStruct.dataOps->typeID) a = NewArray(&floatStruct, tmpDims);
   else if (typeid==doubleStruct.dataOps->typeID) a = NewArray(&doubleStruct, tmpDims);
   else {
      Debug(0, "unsupported typeID\n");
      // fixme - leave nicely
      }

   char *buff= ((Array*) PushDataBlock(a))->value.c;
   memcpy(buff, p_addr, totalnumber * a->type.base->size);

   Debug(5, "   element in array %d\n",totalnumber);
   Debug(5, "   CountDims is %d\n",CountDims(a->type.dims));
   Debug(5, "   typeID %d\n",a->type.base->dataOps->typeID);
   Debug(5, "   numbytes %d\n",totalnumber * a->type.base->size);
      
   int status = shmdt((void*)addr);
   if (status == -1) perror ("shmdt failed");

   unlock_slot(m,slot);
   unlock_master(m);
   detach_master(m);

   return;
   }

//---------------------------------------------------------------
// Y_shm_free
//---------------------------------------------------------------
void Y_shm_free(long key, char* id) {
   
   slot_master *m = attach_master(key);
   if (!m) {
      PushIntValue(-1);
      return;
   } 
   
   lock_master(m);
   
   int slot;
   if ((slot = lkup_slot(m,id)) < 0) {
      Debug(0, "slot not found\n");
      unlock_master(m);
      detach_master(m);
      PushIntValue(-1);
      return;
   }
   
   free_slot(m, slot);

   unlock_master(m);
   detach_master(m);

   PushIntValue(0);
   return;

}

//---------------------------------------------------------------
// Y_shm_cleanup
//---------------------------------------------------------------
void Y_shm_cleanup(long key) {
   int status;
   
   slot_master *m = attach_master(key);
   if (!m) {
      PushIntValue(-1);
      return;
   }


   lock_master(m);

   int i;
   
   for (i=0;i<m->numslots;i++) {
      free_slot(m, i);
   }

   //unlock_master(m);
   status = semctl(m->master_semid,IPC_RMID,0);
   if (status == -1) {
      perror ("semctl IPC_RMID failed");
      PushIntValue(-1);
      return;
   }
   
   status = shmctl(m->master_shmid,IPC_RMID,0);
   if (status == -1) {
      perror ("shmctl IPC_RMID failed");
      PushIntValue(-1);
      return;
   }
   
   detach_master(m);
   
   PushIntValue(0);
   return;

}

//--------------------------------------------------------------------
// shm_var
//--------------------------------------------------------------------

void Y_shm_var(int nArgs)
{
  
   _segm* this;
   void *addr;
   long key = yarg_sl(nArgs-1);
   char *id = yarg_sq(nArgs-2);
   
   // lkup id in segtable
   if ((this=seg_lkupid(segtable,id)) == NULL) {
      slot_master *m = attach_master(key);
      if (!m) {
         YError("master key not found");
         return;
      }

      lock_master(m);

      int slot;
      if ((slot = lkup_slot(m,id)) < 0) {
         unlock_master(m);
         detach_master(m);
         YError("slot not found");
         return;
      }

      lock_slot(m,slot);

      int shmid = m->sse[slot].shmid;

      // attach segment
      addr = (void*)shmat(shmid, NULL, 0);
      if (addr == (void *) -1) {
         unlock_slot(m,slot);
         unlock_master(m);
         detach_master(m);
         YError("slot error");
      } else {
         // append this segment to the local lkup
         this = (_segm*) malloc(sizeof(_segm));
         snprintf(this->id,SLOT_DESC_STRING_MAX,id);
         this->addr=addr;
         this->yaddr=NULL;
         segtable=seg_add(segtable,this);
         // concept of locking is questionable
         // do it now
         unlock_slot(m,slot);
         unlock_master(m);
         detach_master(m);
      }
   } else {
      // already attached
      addr=this->addr;
   }
   
   int typeid = ((int*)addr)[0];
   int countdims = ((int*)addr)[1];
   long totalnumber = 1;

   int *p_addr=(int*)addr+2;

   Dimension *tmp= tmpDims;
   tmpDims= 0;
   FreeDimension(tmp);

   for(;countdims>0;countdims--) {
      totalnumber *= *p_addr;
      tmpDims= NewDimension(*p_addr++, 1L, tmpDims);
   }

   Symbol *arg= sp-nArgs+1;

   // skip over the two args we just parsed
   arg+=2;
   nArgs-=2;

   long index;
  
   if (nArgs<1 || arg->ops!=&referenceSym)
      YError("first argument to reshape must be variable reference");

   index= arg->index;
  
   StructDef *base= 0;
   void *address= 0;
   Array *owner= 0;
   LValue *result;


   address= (char *) p_addr;
   owner= 0;

   if (typeid==charStruct.dataOps->typeID) base = &charStruct;
   else if (typeid==shortStruct.dataOps->typeID) base = &shortStruct;
   else if (typeid==intStruct.dataOps->typeID) base = &intStruct;
   else if (typeid==longStruct.dataOps->typeID) base = &longStruct;
   else if (typeid==floatStruct.dataOps->typeID) base = &floatStruct;
   else if (typeid==doubleStruct.dataOps->typeID) base = &doubleStruct;
   else {
      Debug(0, "fatal: unsupported typeID !!!\n");
      // fixme - leave nicely
      }
            
   if (this->yaddr==NULL) this->yaddr=address;
   Debug(3, "ref established at %x\n",address);
   
   result= PushDataBlock(NewLValueM(owner, address, base, tmpDims));

   PopTo(&globTab[index]);
}

//--------------------------------------------------------------------
// shm_unvar
//--------------------------------------------------------------------

void Y_shm_unvar(int nArgs)
{
   Symbol *arg= sp-nArgs+1;
   long index;
   DataBlock *db;
   if (nArgs!=1 || arg->ops!=&referenceSym)
      YError("shm_unvar argument must be a variable reference");

   index= arg->index;
   db= globTab[index].value.db;  /* might not be meaningful... */

   void *addr = ((LValue *)(globTab[index].value.db))->address.m;
   _segm* this = seg_lkupaddr(segtable,addr);
   if (this == NULL) {
      Debug(0, "no attached mem\n");
   } else {
      Debug(2, "detattach %x\n",this->addr);
      int status = shmdt((void*)this->addr);
      if (status == -1) perror ("shmdt failed");
   }
      

   /* same as var=[], but works for LValues as well */
   globTab[index].value.db= RefNC(&nilDB);
   if (globTab[index].ops==&dataBlockSym) { 
      Unref(db);
      Debug(5, "Unref\n");
   }
   else {
      globTab[index].ops= &dataBlockSym;
      Debug(5, "ok\n");
   }
   Drop(1);
}

