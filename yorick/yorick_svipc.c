/*
 *    Copyright (C) 2011-2012  Matthieu Bec
 *  
 *    This file is part of yp-svipc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "ydata.h"

#include "svipc.h"

//---------------------------------------------------------------
// Y_nprocs
//---------------------------------------------------------------
void Y_nprocs(int nArgs)
{
	PushLongValue(svipc_nprocs());
}

//---------------------------------------------------------------
// Y_getpid
//---------------------------------------------------------------
void Y_getpid(int nArgs)
{
	PushIntValue(getpid());
}

//---------------------------------------------------------------
// Y_fork
//---------------------------------------------------------------

void Y_fork(int nArgs)
{
	pid_t pid;

	// automatic child reaping
	signal(SIGCHLD, SIG_IGN);

	pid = fork();

	if (pid == 0) {
		// the child needs a stdin to keep yorick's eventloop happy,
		// but independent from the parent one. Create a dummy one.
		int fd[2];
		pipe(fd);	// create a dummy pipe
		close(STDIN_FILENO);	// close stdin inherited from fork
		dup2(fd[0], STDIN_FILENO);	// swap in the pipe's read end as new stdin
		close(fd[1]);	// pipe's write end is not used, close it
	}

	PushIntValue(pid);
}

//---------------------------------------------------------------
// Y_ftok
//---------------------------------------------------------------
void Y_ftok(char *path, int proj)
{
	key_t key = svipc_ftok(path, proj);
	PushIntValue(key);
}

//---------------------------------------------------------------
// Y_shm_init
//---------------------------------------------------------------
void Y_shm_init(int key, int numslots)
{
	int status = svipc_shm_init(key, numslots);
	PushIntValue(status);
}

//---------------------------------------------------------------
// Y_shm_cleanup
//---------------------------------------------------------------
void Y_shm_cleanup(int key)
{
	int status = svipc_shm_cleanup(key);
	PushIntValue(status);
}

//---------------------------------------------------------------
// Y_shm_info
//---------------------------------------------------------------
void Y_shm_info(int key, int details)
{
	int status = svipc_shm_info(key, details);
	PushIntValue(status);
}

//---------------------------------------------------------------
// Y_shm_write
//---------------------------------------------------------------
void Y_shm_write(int key, char *id, void *a, int publish)
{
	slot_array arr;

	Array *array = (Array *) Pointee(a);
	int typeid = array->type.base->dataOps->typeID;
	int countdims = CountDims(array->type.dims);

	if (!countdims) {
		Debug(0, "non array type not supported\n");
		PushIntValue(-1);
		return;
	}
	// long totalnumber = TotalNumber(array->type.dims); // also as, array->type.number

	if (typeid == charStruct.dataOps->typeID)
		arr.typeid = SVIPC_CHAR;
	else if (typeid == shortStruct.dataOps->typeID)
		arr.typeid = SVIPC_SHORT;
	else if (typeid == intStruct.dataOps->typeID)
		arr.typeid = SVIPC_INT;
	else if (typeid == longStruct.dataOps->typeID)
		arr.typeid = SVIPC_LONG;
	else if (typeid == floatStruct.dataOps->typeID)
		arr.typeid = SVIPC_FLOAT;
	else if (typeid == doubleStruct.dataOps->typeID)
		arr.typeid = SVIPC_DOUBLE;
	else {
		Debug(0, "type not supported\n");
		PushIntValue(-1);
		return;
	}

	arr.countdims = countdims;
	arr.number = (int *)malloc(arr.countdims * sizeof(*arr.number));
	Dimension *d;
	int *pnum = arr.number;
	for (d = array->type.dims;; d = d->next) {
		*pnum++ = d->number;
		if (d->next == NULL)
			break;
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
void Y_shm_read(int key, char *id, float subscribe)
{
	slot_array arr;

	memset(&arr, 0, sizeof(arr));
	int status = svipc_shm_read(key, id, &arr, subscribe);
	if (status == 0) {
		Dimension *tmp = tmpDims;
		tmpDims = 0;
		FreeDimension(tmp);
		int countdims = arr.countdims;
		int *pnum = arr.number + arr.countdims - 1;
		long totalnumber = 1;
		for (; countdims > 0; countdims--) {
			totalnumber *= *pnum;
			tmpDims = NewDimension(*pnum--, 1L, tmpDims);
		}
		Array *a;
		if (arr.typeid == SVIPC_CHAR)
			a = NewArray(&charStruct, tmpDims);
		else if (arr.typeid == SVIPC_SHORT)
			a = NewArray(&shortStruct, tmpDims);
		else if (arr.typeid == SVIPC_INT)
			a = NewArray(&intStruct, tmpDims);
		else if (arr.typeid == SVIPC_LONG)
			a = NewArray(&longStruct, tmpDims);
		else if (arr.typeid == SVIPC_FLOAT)
			a = NewArray(&floatStruct, tmpDims);
		else if (arr.typeid == SVIPC_DOUBLE)
			a = NewArray(&doubleStruct, tmpDims);
		else {
			release_slot_array(&arr);
			Debug(0, "type not supported\n");
			PushIntValue(-1);
			return;
		}

		char *buff = ((Array *) PushDataBlock(a))->value.c;
		memcpy(buff, arr.data, totalnumber * a->type.base->size);
		release_slot_array(&arr);
	} else {
		Debug(1, "read failed\n");	// debug level 1: could be a timeout
		PushIntValue(-1);
		return;
	}
}

//---------------------------------------------------------------
// Y_shm_free
//---------------------------------------------------------------
void Y_shm_free(int key, char *id)
{
	int status = svipc_shm_free(key, id);
	PushIntValue(status);
}

//--------------------------------------------------------------------
// Y_shm_var
//--------------------------------------------------------------------

void Y_shm_var(int nArgs)
{
	slot_array arr;
	int key = (int)yarg_sl(nArgs - 1);
	char *id = yarg_sq(nArgs - 2);

	int status = svipc_shm_attach(key, id, &arr);
	if (status)
		YError("svipc_shm_attach failed");

	int typeid = arr.typeid;
	int countdims = arr.countdims;

	Dimension *tmp = tmpDims;
	tmpDims = 0;
	FreeDimension(tmp);

	int *pnum = arr.number + arr.countdims - 1;
	for (; countdims > 0; countdims--) {
		tmpDims = NewDimension(*pnum--, 1L, tmpDims);
	}

	Symbol *arg = sp - nArgs + 1;

	// skip over the two args we just parsed
	arg += 2;
	nArgs -= 2;

	long index;

	if (nArgs < 1 || arg->ops != &referenceSym)
		YError("first argument to reshape must be variable reference");

	index = arg->index;

	StructDef *base = 0;
	void *address = 0;
	Array *owner = 0;
	LValue *result;

	address = (char *)arr.data;
	owner = 0;

	if (typeid == charStruct.dataOps->typeID)
		base = &charStruct;
	else if (typeid == shortStruct.dataOps->typeID)
		base = &shortStruct;
	else if (typeid == intStruct.dataOps->typeID)
		base = &intStruct;
	else if (typeid == longStruct.dataOps->typeID)
		base = &longStruct;
	else if (typeid == floatStruct.dataOps->typeID)
		base = &floatStruct;
	else if (typeid == doubleStruct.dataOps->typeID)
		base = &doubleStruct;
	else {
		Debug(0, "fatal: unsupported typeID !!!\n");
		// fixme - leave nicely
	}

	Debug(3, "ref established at pdata %p\n", address);
	result = PushDataBlock(NewLValueM(owner, address, base, tmpDims));

	PopTo(&globTab[index]);
}

//--------------------------------------------------------------------
// Y_shm_unvar
//--------------------------------------------------------------------

void Y_shm_unvar(int nArgs)
{
	Symbol *arg = sp - nArgs + 1;
	long index;
	DataBlock *db;
	if (nArgs != 1 || arg->ops != &referenceSym)
		YError("shm_unvar argument must be a variable reference");

	index = arg->index;
	db = globTab[index].value.db;	/* might not be meaningful... */

	void *addr = ((LValue *) (globTab[index].value.db))->address.m;

	int status = svipc_shm_detach(addr);
	if (status)
		YError("svipc_shm_detach failed");

	/* same as var=[], but works for LValues as well */
	globTab[index].value.db = RefNC(&nilDB);
	if (globTab[index].ops == &dataBlockSym) {
		Unref(db);
		Debug(5, "Unref\n");
	} else {
		globTab[index].ops = &dataBlockSym;
		Debug(5, "ok\n");
	}
	Drop(1);
}

//--------------------------------------------------------------------
// sempahores
//--------------------------------------------------------------------

void Y_sem_init(int key, int numslots)
{
	int status = svipc_sem_init(key, numslots);
	PushIntValue(status);
}

void Y_sem_cleanup(int key)
{
	int status = svipc_sem_cleanup(key);
	PushIntValue(status);
}

void Y_sem_info(int key, int details)
{
	int status = svipc_sem_info(key, details);
	PushIntValue(status);
}

void Y_sem_take(int key, int id, int count, float wait)
{
	int status = svipc_semtake(key, id, count, wait);
	PushIntValue(status);
}

void Y_sem_give(int key, int id, int count)
{
	int status = svipc_semgive(key, id, count);
	PushIntValue(status);
}

//--------------------------------------------------------------------
// message queues
//--------------------------------------------------------------------

void Y_msq_init(int key)
{
	int status = svipc_msq_init(key);
	PushIntValue(status);
}

void Y_msq_cleanup(int key)
{
	int status = svipc_msq_cleanup(key);
	PushIntValue(status);
}

void Y_msq_info(int key, int details)
{
	int status = svipc_msq_info(key, details);
	PushIntValue(status);
}

void Y_msq_snd(int key, long mtype, void *a, int nowait)
{
	Array *array = (Array *) Pointee(a);
	int typeid = array->type.base->dataOps->typeID;
	int countdims = CountDims(array->type.dims);
	long totalnumber = TotalNumber(array->type.dims);

	if (!countdims) {
		Debug(0, "non array type not supported\n");
		PushIntValue(-1);
		return;
	}

	int sizeoftype;

	if (typeid == charStruct.dataOps->typeID)
		sizeoftype = sizeof(char);
	else if (typeid == shortStruct.dataOps->typeID)
		sizeoftype = sizeof(short);
	else if (typeid == intStruct.dataOps->typeID)
		sizeoftype = sizeof(int);
	else if (typeid == longStruct.dataOps->typeID)
		sizeoftype = sizeof(long);
	else if (typeid == floatStruct.dataOps->typeID)
		sizeoftype = sizeof(float);
	else if (typeid == doubleStruct.dataOps->typeID)
		sizeoftype = sizeof(double);
	else {
		Debug(0, "type not supported\n");
		PushIntValue(-1);
		return;
	}

	size_t msgsz =
	    sizeof(typeid) + sizeof(countdims) + countdims * sizeof(countdims) +
	    totalnumber * sizeoftype;

	struct svipc_msgbuf *sendmsg =
	    malloc(sizeof(struct svipc_msgbuf) + msgsz);

	sendmsg->mtype = mtype;

	int *msgp_pint = (int *)sendmsg->mtext;

	*msgp_pint++ = typeid;
	*msgp_pint++ = countdims;
	Dimension *d;
	for (d = array->type.dims;; d = d->next) {
		*msgp_pint++ = d->number;
		if (d->next == NULL)
			break;
	}
	memcpy(msgp_pint, a, totalnumber * sizeoftype);

	Debug(3, "Y_msq_snd typeid %d countdims %d totalnumber %ld\n", typeid,
	      countdims, totalnumber);
	int status = svipc_msq_snd(key, sendmsg, msgsz, nowait);

	free(sendmsg);

	PushIntValue(status);
}

void Y_msq_rcv(int key, long mtype, int nowait)
{
	int *msgp_pint;
	struct svipc_msgbuf *recvmsg;

	int status = svipc_msq_rcv(key, mtype, &recvmsg, nowait);

	msgp_pint = (int *)recvmsg->mtext;

	if (status == 0) {
		Dimension *tmp = tmpDims;
		tmpDims = 0;
		FreeDimension(tmp);
		int typeid = *msgp_pint++;
		status = typeid;
		int countdims = *msgp_pint++;
		long totalnumber = 1;
		int *msgp_pint0 = msgp_pint;
		for (; countdims > 0; countdims--) {
			int thisdim = *(msgp_pint0 + countdims - 1);
			msgp_pint++;
			totalnumber *= thisdim;
			tmpDims = NewDimension(thisdim, 1L, tmpDims);
		}

		Array *a;
		if (typeid == SVIPC_CHAR)
			a = NewArray(&charStruct, tmpDims);
		else if (typeid == SVIPC_SHORT)
			a = NewArray(&shortStruct, tmpDims);
		else if (typeid == SVIPC_INT)
			a = NewArray(&intStruct, tmpDims);
		else if (typeid == SVIPC_LONG)
			a = NewArray(&longStruct, tmpDims);
		else if (typeid == SVIPC_FLOAT)
			a = NewArray(&floatStruct, tmpDims);
		else if (typeid == SVIPC_DOUBLE)
			a = NewArray(&doubleStruct, tmpDims);
		else {
			Debug(0, "type not supported\n");
			PushIntValue(-1);
			return;
		}

		char *buff = ((Array *) PushDataBlock(a))->value.c;
		memcpy(buff, msgp_pint, totalnumber * a->type.base->size);

		// cleanup
		free(recvmsg);

	} else {
		PushIntValue(status);
	}
}
