#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ydata.h"

#include "svipc.h"


//---------------------------------------------------------------
// ftok
//---------------------------------------------------------------
long Y_ftok(char *path, int proj) {
   long key = svipc_ftok(path, proj);
   
   PushLongValue(key);

}

//---------------------------------------------------------------
// Y_shm_info
//---------------------------------------------------------------
void Y_shm_info(long key, long details) {
   int status = svipc_shm_info(key,details);
   
   PushIntValue(status);
   
}

//---------------------------------------------------------------
// Y_shm_init
//---------------------------------------------------------------
void Y_shm_init(long key, long numslots) {
   
   int status = svipc_shm_init(key, numslots);
   
   PushIntValue(status);
   
}

//---------------------------------------------------------------
// Y_shm_write
//---------------------------------------------------------------
void Y_shm_write(long key, char *id, void *a, int publish) {
   slot_array arr;
   
   Array *array= (Array *)Pointee(a);
   int typeid = array->type.base->dataOps->typeID;
   int countdims = CountDims(array->type.dims);
   long totalnumber = TotalNumber(array->type.dims); // also as, array->type.number

   if (typeid==charStruct.dataOps->typeID) arr.typeid = SVIPC_CHAR;
   else if (typeid==shortStruct.dataOps->typeID) arr.typeid = SVIPC_SHORT;
   else if (typeid==intStruct.dataOps->typeID) arr.typeid = SVIPC_INT;
   else if (typeid==longStruct.dataOps->typeID) arr.typeid = SVIPC_LONG;
   else if (typeid==floatStruct.dataOps->typeID) arr.typeid = SVIPC_FLOAT;
   else if (typeid==doubleStruct.dataOps->typeID) arr.typeid = SVIPC_DOUBLE;
   else {
      Debug(0, "type not supported\n");
      PushIntValue(-1);
      return;
   }
   
   arr.countdims = countdims;
   arr.number = (int*) malloc(arr.countdims * sizeof(*arr.number));
   Dimension *d;
   int *pnum = arr.number;
   for (d=array->type.dims;;d=d->next) {
      *pnum++=d->number;
      if (d->next==NULL) break;
   }
   arr.data = a;
   
   int status = svipc_shm_write(key, id, &arr, publish);
   PushIntValue(status);
   return;
   }

//---------------------------------------------------------------
// Y_shm_read
//---------------------------------------------------------------
void Y_shm_read(long key, char *id, int subscribe) {
   slot_array arr;
   
   memset(&arr,0, sizeof(arr));
   int status = svipc_shm_read(key, id, &arr, subscribe);
   if (status==0) {
      Dimension *tmp= tmpDims;
      tmpDims= 0;
      FreeDimension(tmp);
      int countdims = arr.countdims;
      int *pnum = arr.number;
      long totalnumber = 1;
      for(;countdims>0;countdims--) {
         totalnumber *= *pnum;
         tmpDims= NewDimension(*pnum++, 1L, tmpDims);
      }
      Array *a;
      if (arr.typeid==SVIPC_CHAR) a = NewArray(&charStruct, tmpDims);
      else if (arr.typeid==SVIPC_SHORT) a = NewArray(&shortStruct, tmpDims);
      else if (arr.typeid==SVIPC_INT) a = NewArray(&intStruct, tmpDims);
      else if (arr.typeid==SVIPC_LONG) a = NewArray(&longStruct, tmpDims);
      else if (arr.typeid==SVIPC_FLOAT) a = NewArray(&floatStruct, tmpDims);
      else if (arr.typeid==SVIPC_DOUBLE) a = NewArray(&doubleStruct, tmpDims);
      else {
         free(arr.number);
         Debug(0, "type not supported\n");
         PushIntValue(-1);
         return;
         }
      
      char *buff= ((Array*) PushDataBlock(a))->value.c;
      memcpy(buff, arr.data, totalnumber * a->type.base->size);
   } else {
      Debug(0, "read failed\n");
      PushIntValue(-1);
      return;
	}
}

//---------------------------------------------------------------
// Y_shm_free
//---------------------------------------------------------------
void Y_shm_free(long key, char* id) {
   
   int status = svipc_shm_free(key, id);
   
   PushIntValue(status);

}

//---------------------------------------------------------------
// Y_shm_cleanup
//---------------------------------------------------------------
void Y_shm_cleanup(long key) {

   int status = svipc_shm_cleanup(key);
   PushIntValue(status);

}

//--------------------------------------------------------------------
// shm_var
//--------------------------------------------------------------------

void Y_shm_var(int nArgs)
{
//   
//    _segm* this;
//    void *addr;
//    long key = yarg_sl(nArgs-1);
//    char *id = yarg_sq(nArgs-2);
//    
//    // lkup id in segtable
//    if ((this=seg_lkupid(segtable,id)) == NULL) {
//       slot_master *m = attach_master(key);
//       if (!m) {
//          YError("master key not found");
//          return;
//       }
// 
//       lock_master(m);
// 
//       int slot;
//       if ((slot = lkup_slot(m,id)) < 0) {
//          unlock_master(m);
//          detach_master(m);
//          YError("slot not found");
//          return;
//       }
// 
//       lock_slot(m,slot);
// 
//       int shmid = m->sse[slot].shmid;
// 
//       // attach segment
//       addr = (void*)shmat(shmid, NULL, 0);
//       if (addr == (void *) -1) {
//          unlock_slot(m,slot);
//          unlock_master(m);
//          detach_master(m);
//          YError("slot error");
//       } else {
//          // append this segment to the local lkup
//          this = (_segm*) malloc(sizeof(_segm));
//          snprintf(this->id,SLOT_DESC_STRING_MAX,id);
//          this->addr=addr;
//          this->yaddr=NULL;
//          segtable=seg_add(segtable,this);
//          // concept of locking is questionable
//          // do it now
//          unlock_slot(m,slot);
//          unlock_master(m);
//          detach_master(m);
//       }
//    } else {
//       // already attached
//       addr=this->addr;
//    }
//    
//    int typeid = ((int*)addr)[0];
//    int countdims = ((int*)addr)[1];
//    long totalnumber = 1;
// 
//    long *p_addr=(long*)((int*)addr+2);
// 
//    Dimension *tmp= tmpDims;
//    tmpDims= 0;
//    FreeDimension(tmp);
// 
//    for(;countdims>0;countdims--) {
//       totalnumber *= *p_addr;
//       tmpDims= NewDimension(*p_addr++, 1L, tmpDims);
//    }
// 
//    Symbol *arg= sp-nArgs+1;
// 
//    // skip over the two args we just parsed
//    arg+=2;
//    nArgs-=2;
// 
//    long index;
//   
//    if (nArgs<1 || arg->ops!=&referenceSym)
//       YError("first argument to reshape must be variable reference");
// 
//    index= arg->index;
//   
//    StructDef *base= 0;
//    void *address= 0;
//    Array *owner= 0;
//    LValue *result;
// 
// 
//    address= (char *) p_addr;
//    owner= 0;
// 
//    if (typeid==charStruct.dataOps->typeID) base = &charStruct;
//    else if (typeid==shortStruct.dataOps->typeID) base = &shortStruct;
//    else if (typeid==intStruct.dataOps->typeID) base = &intStruct;
//    else if (typeid==longStruct.dataOps->typeID) base = &longStruct;
//    else if (typeid==floatStruct.dataOps->typeID) base = &floatStruct;
//    else if (typeid==doubleStruct.dataOps->typeID) base = &doubleStruct;
//    else {
//       Debug(0, "fatal: unsupported typeID !!!\n");
//       // fixme - leave nicely
//       }
//             
//    if (this->yaddr==NULL) this->yaddr=address;
//    Debug(3, "ref established at %x\n",address);
//    
//    result= PushDataBlock(NewLValueM(owner, address, base, tmpDims));
// 
//    PopTo(&globTab[index]);
}

//--------------------------------------------------------------------
// shm_unvar
//--------------------------------------------------------------------

void Y_shm_unvar(int nArgs)
{
//    Symbol *arg= sp-nArgs+1;
//    long index;
//    DataBlock *db;
//    if (nArgs!=1 || arg->ops!=&referenceSym)
//       YError("shm_unvar argument must be a variable reference");
// 
//    index= arg->index;
//    db= globTab[index].value.db;  /* might not be meaningful... */
// 
//    void *addr = ((LValue *)(globTab[index].value.db))->address.m;
//    _segm* this = seg_lkupaddr(segtable,addr);
//    if (this == NULL) {
//       Debug(0, "no attached mem\n");
//    } else {
//       Debug(2, "detattach %x\n",this->addr);
//       int status = shmdt((void*)this->addr);
//       if (status == -1) perror ("shmdt failed");
//    }
//       
// 
//    /* same as var=[], but works for LValues as well */
//    globTab[index].value.db= RefNC(&nilDB);
//    if (globTab[index].ops==&dataBlockSym) { 
//       Unref(db);
//       Debug(5, "Unref\n");
//    }
//    else {
//       globTab[index].ops= &dataBlockSym;
//       Debug(5, "ok\n");
//    }
//    Drop(1);
}

