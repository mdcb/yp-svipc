#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ydata.h"

#include "svipc.h"


//---------------------------------------------------------------
// Y_fork
//---------------------------------------------------------------
void Y_fork(int nArgs)
{
  pid_t   pid;
  int     fd[2];

  pipe(fd);
  
  pid = fork();

  if (pid==0) {
    // close child stdin
    close(0);
    // replace it by a bogus fd else yorick child dies.
    dup(fd[0]);
  }

  PushLongValue((long)pid);
}


//---------------------------------------------------------------
// Y_ftok
//---------------------------------------------------------------
void Y_ftok(char *path, int proj) {
  long key = svipc_ftok(path, proj);
   
  PushLongValue(key);
}

//---------------------------------------------------------------
// Y_shm_init
//---------------------------------------------------------------
void Y_shm_init(long key, long numslots) {
   
  int status = svipc_shm_init(key, numslots);
   
  PushIntValue(status);
   
}

//---------------------------------------------------------------
// Y_shm_cleanup
//---------------------------------------------------------------
void Y_shm_cleanup(long key) {

  int status = svipc_shm_cleanup(key);
  PushIntValue(status);

}

//---------------------------------------------------------------
// Y_shm_info
//---------------------------------------------------------------
void Y_shm_info(long key, long details) {
  int status = svipc_shm_info(key,details);
   
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
  // long totalnumber = TotalNumber(array->type.dims); // also as, array->type.number

  if      (typeid==charStruct.dataOps->typeID)   arr.typeid = SVIPC_CHAR;
  else if (typeid==shortStruct.dataOps->typeID)  arr.typeid = SVIPC_SHORT;
  else if (typeid==intStruct.dataOps->typeID)    arr.typeid = SVIPC_INT;
  else if (typeid==longStruct.dataOps->typeID)   arr.typeid = SVIPC_LONG;
  else if (typeid==floatStruct.dataOps->typeID)  arr.typeid = SVIPC_FLOAT;
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
  
  // fixme: cleanup the api
  free(arr.number);
  
  PushIntValue(status);
  return;
}

//---------------------------------------------------------------
// Y_shm_read
//---------------------------------------------------------------
void Y_shm_read(long key, char *id, float subscribe) {
  slot_array arr;
   
  memset(&arr,0, sizeof(arr));
  int status = svipc_shm_read(key, id, &arr, subscribe);
  if (status==0) {
    Dimension *tmp= tmpDims;
    tmpDims= 0;
    FreeDimension(tmp);
    int countdims = arr.countdims;
    int *pnum = arr.number+arr.countdims-1;
    long totalnumber = 1;
    for(;countdims>0;countdims--) {
      totalnumber *= *pnum;
      tmpDims= NewDimension(*pnum--, 1L, tmpDims);
    }
    Array *a;
    if (arr.typeid==SVIPC_CHAR) a = NewArray(&charStruct, tmpDims);
    else if (arr.typeid==SVIPC_SHORT) a = NewArray(&shortStruct, tmpDims);
    else if (arr.typeid==SVIPC_INT) a = NewArray(&intStruct, tmpDims);
    else if (arr.typeid==SVIPC_LONG) a = NewArray(&longStruct, tmpDims);
    else if (arr.typeid==SVIPC_FLOAT) a = NewArray(&floatStruct, tmpDims);
    else if (arr.typeid==SVIPC_DOUBLE) a = NewArray(&doubleStruct, tmpDims);
    else {
      release_slot_array(&arr);
      Debug(0, "type not supported\n");
      PushIntValue(-1);
      return;
    }
      
    char *buff= ((Array*) PushDataBlock(a))->value.c;
    memcpy(buff, arr.data, totalnumber * a->type.base->size);
    release_slot_array(&arr);
  } else {
    Debug(1, "read failed\n"); // debug level 1: could be a timeout
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

//--------------------------------------------------------------------
// Y_shm_var
//--------------------------------------------------------------------

void Y_shm_var(int nArgs)
{
  slot_array arr;
  long key = yarg_sl(nArgs-1);
  char *id = yarg_sq(nArgs-2);
   
  int status = svipc_shm_attach(key,id,&arr);
  if (status)
    YError("svipc_shm_attach failed");
   
  int typeid = arr.typeid;
  int countdims = arr.countdims;
   
  int *p_addr=arr.number;

  Dimension *tmp= tmpDims;
  tmpDims= 0;
  FreeDimension(tmp);

  for(;countdims>0;countdims--) {
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


  address= (char *) arr.data;
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
            
  Debug(3, "ref established\n");
   
  result= PushDataBlock(NewLValueM(owner, address, base, tmpDims));

  PopTo(&globTab[index]);
}

//--------------------------------------------------------------------
// Y_shm_unvar
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
      
  int status = svipc_shm_detach(addr);
  if (status)
    YError("svipc_shm_detach failed");
   
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


//--------------------------------------------------------------------
// sempahores
//--------------------------------------------------------------------

void Y_sem_init(long key, long numslots)
{
  int status = svipc_sem_init(key, numslots);
  PushIntValue(status);
}

void Y_sem_cleanup(long key) {
  int status = svipc_sem_cleanup(key);
  PushIntValue(status);
}

void Y_sem_info(long key, long details)
{
  int status = svipc_sem_info(key, details);
  PushIntValue(status);
}

void Y_sem_take(long key,long id,float wait)
{
  int status = svipc_semtake(key, id, wait);
  PushIntValue(status);
}

void Y_sem_give(long key,long id)
{
  int status = svipc_semgive(key, id);
  PushIntValue(status);
}

