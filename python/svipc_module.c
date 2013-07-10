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

#define NPY_NO_DEPRECATED_API 7

#include "Python.h"
#include <numpy/arrayobject.h>

#if PY_MAJOR_VERSION >= 3
#define PyInt_FromLong PyLong_FromLong
#endif

#include <stdio.h>
#include <string.h>

#include "svipc.h"

PyObject *python_svipc_module;
PyObject *python_svipc_error;

#define PYTHON_SVIPC_USAGE(fmt, ...) {\
   PyErr_Format(python_svipc_error, "usage: " fmt, ## __VA_ARGS__);\
   return NULL;\
   }

#define PYTHON_SVIPC_ERROR(fmt, ...) {\
   PyErr_Format(python_svipc_error, fmt, ## __VA_ARGS__);\
   return NULL;\
   }

/*******************************************************************
 * setaffinity
 *******************************************************************/
PyDoc_STRVAR(python_svipc_misc_setaffinity_doc, "setaffinity(cpu=cpu)\n\
  (int)    cpu - cpu id\n\
Set the running process affinity to cpu.\n\
");

PyObject *python_svipc_misc_setaffinity(PyObject * self, PyObject * args,
				 PyObject * kwds)
{
	static char *kwlist[] = { "cpu", NULL };
	int cpu=0;

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "i", kwlist, &cpu))
		PYTHON_SVIPC_USAGE("setaffinity(cpu=cpu)");

	int status = svipc_setaffinity(cpu);

	return PyInt_FromLong(status);
}

/*******************************************************************
 * ftok
 *******************************************************************/
PyDoc_STRVAR(python_svipc_misc_ftok_doc, "ftok(path, proj=0)\n\
  (string) path - a unix file path\n\
  (int)    proj - a project number\n\
Convert a pathname and a project identifier to a System V IPC key\n\
");

PyObject *python_svipc_misc_ftok(PyObject * self, PyObject * args,
				 PyObject * kwds)
{

	char *path;
	static char *kwlist[] = { "path", "proj", NULL };
	int proj=0;

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "s|i", kwlist, &path, &proj))
		PYTHON_SVIPC_USAGE("ftok(path, proj=0)");

	long key = svipc_ftok(path, proj);

	return PyInt_FromLong(key);
}

/*******************************************************************
 * nprocs
 *******************************************************************/
PyDoc_STRVAR(python_svipc_misc_nprocs_doc, "nprocs()\n\
Returns the number of processors currently online (available).\n\
Note this might not catch virtualized CPUs.\n\
");

PyObject *python_svipc_misc_nprocs(PyObject * self, PyObject * args)
{
	return PyInt_FromLong(svipc_nprocs());
}

/*******************************************************************
 * shm info
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_info_doc, "shm_info(key, details=0)\n\
  (int) key     - a System V IPC key\n\
  (int) details - the level of details to print\n\
Print a report on shared memory pool identified by 'key'.\n\
'details' controls the level of information printed out.\n\
");

PyObject *python_svipc_shm_info(PyObject * self, PyObject * args,
				PyObject * kwds)
{
	int key, details = 0;

	static char *kwlist[] = { "key", "details", NULL };

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "i|i", kwlist, &key, &details))
		PYTHON_SVIPC_USAGE("shm_info(key, details=0)");

	int status = svipc_shm_info(key, details);

	return PyInt_FromLong(status);
}

/*******************************************************************
 * shm init
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_init_doc, "shm_init(key, slots)\n\
  (int) key   - a System V IPC key\n\
  (int) slots - the number of shared memory segments to create\n\
Initialize a pool of shared memory identified by 'key' containing\n\
'slots' segments of initially free Ids\n\
");

PyObject *python_svipc_shm_init(PyObject * self, PyObject * args,
				PyObject * kwds)
{
	int key, numslots = -1;

	static char *kwlist[] = { "key", "slots", NULL };

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "ii", kwlist, &key, &numslots))
		PYTHON_SVIPC_USAGE("shm_init(key, slots)");

	int status = svipc_shm_init(key, numslots);

	return PyInt_FromLong(status);
}

/*******************************************************************
 * shm write
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_write_doc, "shm_write(key,id,data,publish=0)\n\
  (int)    key     - a System V IPC key\n\
  (string) id      - a slot Id\n\
  (object) data    - data to be written\n\
  (bool)   publish - broadcast to subscribers a new value has been written\n\
Write the content of the variable referenced by a in\n\
the slot identified by 'id' from the shared memory pool\n\
identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency for external readers.\n\
");

PyObject *python_svipc_shm_write(PyObject * self, PyObject * args,
				 PyObject * kwds)
{

	int key;
	char *id;
	PyObject *a;
	int publish = 0;

	static char *kwlist[] = { "key", "id", "data", "publish", NULL };

	slot_array arr;

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "isO|i", kwlist, &key, &id, &a, &publish))
		PYTHON_SVIPC_USAGE("shm_write(key,id,data,publish=0)");

	PyArrayObject *inp_array = (PyArrayObject *) PyArray_FROM_O(a);

	if (PyArray_TYPE(inp_array) == NPY_BYTE)
		arr.typeid = SVIPC_CHAR;
	else if (PyArray_TYPE(inp_array) == NPY_SHORT)
		arr.typeid = SVIPC_SHORT;
	else if (PyArray_TYPE(inp_array) == NPY_INT)
		arr.typeid = SVIPC_INT;
	else if (PyArray_TYPE(inp_array) == NPY_LONG)
		arr.typeid = SVIPC_LONG;
	else if (PyArray_TYPE(inp_array) == NPY_FLOAT)
		arr.typeid = SVIPC_FLOAT;
	else if (PyArray_TYPE(inp_array) == NPY_DOUBLE)
		arr.typeid = SVIPC_DOUBLE;
	else {
		PYTHON_SVIPC_ERROR("type not supported");
	}

	arr.countdims = PyArray_NDIM(inp_array);
	arr.number = (int *)malloc(arr.countdims * sizeof(*arr.number));
	memcpy(arr.number, PyArray_DIMS(inp_array),
	       arr.countdims * sizeof(*arr.number));

	arr.data = PyArray_DATA(inp_array);

	int status = svipc_shm_write(key, id, &arr, publish);

	free(arr.number);
	Py_DECREF(inp_array);

	return PyInt_FromLong(status);

}

/*******************************************************************
 * shm read
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_read_doc, "shm_read(key,id,subscribe=0)\n\
  (int)    key       - a System V IPC key\n\
  (string) id        - a slot Id\n\
  (float)  subscribe - if set, wait (block) for a publisher broadcast\n\
Read the content of the slot identified by 'id' from the\n\
shared memory pool identified by 'key'.\n\
  If subscribe > 0, the parameter is understood as a maximum number of\n\
  seconds to wait for a broadcast event, or timeout.\n\
\n\
  If subscribe < 0, the calling process will block until reception of\n\
  a broadcast.\n\
\n\
  If subscribe = 0, read the current value from shared memory\n\
  indepently of write broadcast.\n\
This operation is semaphore protected and guarantees\n\
consistency with external writers.\n\
");

PyObject *python_svipc_shm_read(PyObject * self, PyObject * args,
				PyObject * kwds)
{
	int key;
	char *id;
	slot_array arr;
	float subscribe = 0.0;

	static char *kwlist[] = { "key", "id", "subscribe", NULL };

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "is|f", kwlist, &key, &id, &subscribe))
		PYTHON_SVIPC_USAGE("shm_read(key, id,subscribe=0)");

	memset(&arr, 0, sizeof(arr));
	int status = svipc_shm_read(key, id, &arr, subscribe);

	if (status == 0) {
		enum NPY_TYPES ret_py_type;
		if (arr.typeid == SVIPC_CHAR)
			ret_py_type = NPY_BYTE;
		else if (arr.typeid == SVIPC_SHORT)
			ret_py_type = NPY_SHORT;
		else if (arr.typeid == SVIPC_INT)
			ret_py_type = NPY_INT;
		else if (arr.typeid == SVIPC_LONG)
			ret_py_type = NPY_LONG;
		else if (arr.typeid == SVIPC_FLOAT)
			ret_py_type = NPY_FLOAT;
		else if (arr.typeid == SVIPC_DOUBLE)
			ret_py_type = NPY_DOUBLE;
		else {
			release_slot_array(&arr);
			PYTHON_SVIPC_ERROR("type not supported");
		};

		/* platform ints for numpy array dims */
		npy_intp *dims = malloc(arr.countdims * sizeof(npy_intp));
		int i;
		for (i = 0; i < arr.countdims; i++)
			dims[i] = arr.number[i];

		PyArrayObject *res =
		    (PyArrayObject *) PyArray_SimpleNewFromData(arr.countdims,
								dims,
								ret_py_type,
								arr.data);

		// free tmp 
		free(dims);
    
    // to save us from copying the result, hand over the data to python
    // the slot_array dims though are not used and should be free.
    // sounds weird? trust me. arguably my API could be improved a bit.
    PyArray_ENABLEFLAGS(res,NPY_ARRAY_OWNDATA);
    free(arr.number); // yes

		return (PyObject *) res;
	} else {
		PYTHON_SVIPC_ERROR("status %d\n", status);
	}
}

/*******************************************************************
 * shm free
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_free_doc, "shm_free(key,id)\n\
  (int)    key - a System V IPC key\n\
  (string) id  - a slot Id\n\
Release the slot identified by 'id' from the\n\
shared memory pool identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency with external readers and writers.\n\
");

PyObject *python_svipc_shm_free(PyObject * self, PyObject * args,
				PyObject * kwds)
{

	int key;
	char *id;

	static char *kwlist[] = { "key", "id", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "is", kwlist, &key, &id))
		PYTHON_SVIPC_USAGE("shm_free(key, id)");

	int status = svipc_shm_free(key, id);

	return PyInt_FromLong(status);

}

/*******************************************************************
 * shm cleanup
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_cleanup_doc, "shm_cleanup(key)\n\
  (int) key - a System V IPC key\n\
Release all the slots from the shared memory pool\n\
identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency with external readers and writers.\n\
");

PyObject *python_svipc_shm_cleanup(PyObject * self, PyObject * args,
				   PyObject * kwds)
{

	int key;

	static char *kwlist[] = { "key", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &key))
		PYTHON_SVIPC_USAGE("shm_cleanup(key)");

	int status = svipc_shm_cleanup(key);

	return PyInt_FromLong(status);

}

/*******************************************************************
 * sem info
 *******************************************************************/
PyDoc_STRVAR(python_svipc_sem_info_doc, "sem_info(key, details=0)\n\
  (int) key - a System V IPC key\n\
  (int) details - the level of details to print\n\
Print a report on semaphore pool identified by 'key'.\n\
'details' controls the level of information printed out.\n\
");

PyObject *python_svipc_sem_info(PyObject * self, PyObject * args,
				PyObject * kwds)
{
	int key;
	int details = 0;

	static char *kwlist[] = { "key", "details", NULL };

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "i|i", kwlist, &key, &details))
		PYTHON_SVIPC_USAGE("sem_info(key, details=0)");

	int status = svipc_sem_info(key, details);

	return PyInt_FromLong(status);
}

/*******************************************************************
 * sem init
 *******************************************************************/
PyDoc_STRVAR(python_svipc_sem_init_doc, "sem_init(key, nums)\n\
  (int) key - a System V IPC key\n\
  (int) num - the number of semaphores to create\n\
Initialize a pool of semaphores identified by 'key' containing\n\
'nums' initially taken (locked) semaphores.\n\
NB: nums=0 provides a hacked functionality and reset to 0 all the semaphores\n\
in the pool.\n\
nums<0 is equivalent to sem_info.\n\
");

PyObject *python_svipc_sem_init(PyObject * self, PyObject * args,
				PyObject * kwds)
{
	int key, nums;

	static char *kwlist[] = { "key", "nums", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, &key, &nums))
		PYTHON_SVIPC_USAGE("sem_init(key, nums)");

	int status = svipc_sem_init(key, nums);

	return PyInt_FromLong(status);
}

/*******************************************************************
 * sem cleanup
 *******************************************************************/
PyDoc_STRVAR(python_svipc_sem_cleanup_doc, "sem_cleanup(key)\n\
  (int) key - a System V IPC key\n\
Release the semaphore pool identified by 'key'.\n\
");

PyObject *python_svipc_sem_cleanup(PyObject * self, PyObject * args,
				   PyObject * kwds)
{

	int key;

	static char *kwlist[] = { "key", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &key))
		PYTHON_SVIPC_USAGE("sem_cleanup(key)");

	int status = svipc_sem_cleanup(key);

	return PyInt_FromLong(status);

}

/*******************************************************************
 * sem take
 *******************************************************************/
PyDoc_STRVAR(python_svipc_sem_take_doc, "sem_take(key,id,count=1,wait=-1)\n\
  (int) key    - a System V IPC key\n\
  (int) id     - a semaphore Id\n\
  (int) count  - the number of operations on the semaphore\n\
  (float) wait - a number of seconds\n\
\n\
Decrement semaphore Id by 'count'\n\
The default, count=1, is equivalent to 'take semaphore Id'.\n\
\n\
If wait > 0, the parameter is understood as the maximum number of\n\
seconds to wait to get hold of the semaphore, or timeout.\n\
\n\
If wait < 0, the calling process will block until it can take the\n\
semaphore.\n\
\n\
If wait = 0, returns immediately with a status if the operation\n\
succeeded or not.\n\
");

PyObject *python_svipc_semtake(PyObject * self, PyObject * args,
			       PyObject * kwds)
{

	int key, id, count;
	float wait = -1;
	count = 1;

	static char *kwlist[] = { "key", "id", "count", "wait", NULL };

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "ii|if", kwlist, &key, &id, &count, &wait))
		PYTHON_SVIPC_USAGE("sem_take(key,id,count=1,wait=-1)");

	int status = svipc_semtake(key, id, count, wait);

	return PyInt_FromLong(status);

}

/*******************************************************************
 * sem give
 *******************************************************************/
PyDoc_STRVAR(python_svipc_sem_give_doc, "sem_give(key,id,count=1)\n\
  (int) key   - a System V IPC key\n\
  (int) id    - a semaphore Id\n\
  (int) count - the number of operations on the semaphore\n\
\n\
Increment the semaphore Id by 'count'\n\
The default, count=1, is equivalent to 'release semaphore Id'.\n\
");

PyObject *python_svipc_semgive(PyObject * self, PyObject * args,
			       PyObject * kwds)
{

	int key, id, count;
	count = 1;

	static char *kwlist[] = { "key", "id", "count", NULL };

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "ii|i", kwlist, &key, &id, &count))
		PYTHON_SVIPC_USAGE("sem_give(key,id,count=1)");

	int status = svipc_semgive(key, id, count);

	return PyInt_FromLong(status);

}

/*******************************************************************
 * msq info
 *******************************************************************/
PyDoc_STRVAR(python_svipc_msq_info_doc, "msq_info(key, details=0)\n\
  (int) key - a System V IPC key\n\
  (int) details - the level of details to print\n\
Print a report on message queue identified by 'key'.\n\
'details' controls the level of information printed out.\n\
");

PyObject *python_svipc_msq_info(PyObject * self, PyObject * args,
				PyObject * kwds)
{
	int key;
	int details = 0;

	static char *kwlist[] = { "key", "details", NULL };

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "i|i", kwlist, &key, &details))
		PYTHON_SVIPC_USAGE("msq_info(key, details=0)");

	int status = svipc_msq_info(key, details);

	return PyInt_FromLong(status);
}

/*******************************************************************
 * msq init
 *******************************************************************/
PyDoc_STRVAR(python_svipc_msq_init_doc, "msq_init(key)\n\
  (int) key - a System V IPC key\n\
Initialize a message queue identified by 'key'.\n\
");

PyObject *python_svipc_msq_init(PyObject * self, PyObject * args,
				PyObject * kwds)
{
	int key;

	static char *kwlist[] = { "key", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &key))
		PYTHON_SVIPC_USAGE("msq_init(key)");

	int status = svipc_msq_init(key);

	return PyInt_FromLong(status);
}

/*******************************************************************
 * msq cleanup
 *******************************************************************/
PyDoc_STRVAR(python_svipc_msq_cleanup_doc, "msq_cleanup(key)\n\
  (int) key - a System V IPC key\n\
Release the message queue identified by 'key'.\n\
");

PyObject *python_svipc_msq_cleanup(PyObject * self, PyObject * args,
				   PyObject * kwds)
{

	int key;

	static char *kwlist[] = { "key", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &key))
		PYTHON_SVIPC_USAGE("msq_cleanup(key)");

	int status = svipc_msq_cleanup(key);

	return PyInt_FromLong(status);

}

/*******************************************************************
 * msq snd
 *******************************************************************/
PyDoc_STRVAR(python_svipc_msq_snd_doc, "msq_snd(key,mtype,data,nowait=0)\n\
  (int) key     - a System V IPC key\n\
  (long) mtype  - a message type id\n\
  (object) data - a python object\n\
  (bool) nowait - a boolean\n\
\n\
Sends the content of the variable referenced by a to the message\n\
queue identified by 'key' with a message type of 'mtype'.\n\
\n\
The nowait flag controls if the execution should wait until there\n\
is space in the message queue to send the message or return with an\n\
error.\n\
");

PyObject *python_svipc_msqsnd(PyObject * self, PyObject * args, PyObject * kwds)
{
	int key;
	int mtype;
	PyObject *a;
	int nowait = 0;

	static char *kwlist[] = { "key", "mtype", "data", "nowait", NULL };

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "iiO|i", kwlist, &key, &mtype, &a, &nowait))
		PYTHON_SVIPC_USAGE("msq_snd(key,mtype,data,nowait=0)");

	PyArrayObject *inp_array = (PyArrayObject *) PyArray_FROM_O(a);

	int typeid;

	if (PyArray_TYPE(inp_array) == NPY_BYTE)
		typeid = SVIPC_CHAR;
	else if (PyArray_TYPE(inp_array) == NPY_SHORT)
		typeid = SVIPC_SHORT;
	else if (PyArray_TYPE(inp_array) == NPY_INT)
		typeid = SVIPC_INT;
	else if (PyArray_TYPE(inp_array) == NPY_LONG)
		typeid = SVIPC_LONG;
	else if (PyArray_TYPE(inp_array) == NPY_FLOAT)
		typeid = SVIPC_FLOAT;
	else if (PyArray_TYPE(inp_array) == NPY_DOUBLE)
		typeid = SVIPC_DOUBLE;
	else {
		PYTHON_SVIPC_ERROR("type not supported");
	}

	int sizeoftype = PyArray_ITEMSIZE(inp_array);
	int countdims = PyArray_NDIM(inp_array);
	long totalnumber = PyArray_SIZE(inp_array);

	size_t msgsz =
	    sizeof(typeid) + sizeof(countdims) + countdims * sizeof(countdims) +
	    totalnumber * sizeoftype;
	struct svipc_msgbuf *sendmsg =
	    malloc(sizeof(struct svipc_msgbuf) + msgsz);

	sendmsg->mtype = mtype;

	int *msgp_pint = (int *)sendmsg->mtext;
	*msgp_pint++ = typeid;
	*msgp_pint++ = countdims;
	int i;
	for (i = 0; i < countdims; i++) {
		*msgp_pint++ = *((int *)PyArray_DIMS(inp_array) + i);
	}
	memcpy(msgp_pint, PyArray_DATA(inp_array), totalnumber * sizeoftype);

	int status = svipc_msq_snd(key, sendmsg, msgsz, nowait);

	free(sendmsg);
	Py_DECREF(inp_array);

	return PyInt_FromLong(status);
}

/*******************************************************************
 * msq rcv
 *******************************************************************/
PyDoc_STRVAR(python_svipc_msq_rcv_doc, "msq_rcv(key,mtype,nowait=0)\n\
  (int) key     - a System V IPC key\n\
  (long) mtype  - a message type id\n\
  (bool) nowait - a boolean\n\
Receive a message to queue identified by 'key'.\n\
");

PyObject *python_svipc_msqrcv(PyObject * self, PyObject * args, PyObject * kwds)
{

	int key;
	int mtype;
	int nowait = 0;

	static char *kwlist[] = { "key", "mtype", "nowait", NULL };

	if (!PyArg_ParseTupleAndKeywords
	    (args, kwds, "ii|i", kwlist, &key, &mtype, &nowait))
		PYTHON_SVIPC_USAGE("msq_rcv(key,mtype,nowait=0)");

	int *msgp_pint;
	struct svipc_msgbuf *recvmsg;

	int status = svipc_msq_rcv(key, mtype, &recvmsg, nowait);

	if (status == 0) {
		msgp_pint = (int *)recvmsg->mtext;
		int typeid = *msgp_pint++;
		int countdims = *msgp_pint++;
		int *pdims = msgp_pint;
		int *data = pdims + countdims;
		enum NPY_TYPES ret_py_type;
		if (typeid == SVIPC_CHAR)
			ret_py_type = NPY_BYTE;
		else if (typeid == SVIPC_SHORT)
			ret_py_type = NPY_SHORT;
		else if (typeid == SVIPC_INT)
			ret_py_type = NPY_INT;
		else if (typeid == SVIPC_LONG)
			ret_py_type = NPY_LONG;
		else if (typeid == SVIPC_FLOAT)
			ret_py_type = NPY_FLOAT;
		else if (typeid == SVIPC_DOUBLE)
			ret_py_type = NPY_DOUBLE;
		else {
			free(recvmsg);
			PYTHON_SVIPC_ERROR("type not supported");
		};

		/* platform ints for numpy array dims */
		npy_intp *dims = malloc(countdims * sizeof(npy_intp));
		int i;
		for (i = 0; i < countdims; i++)
			dims[i] = pdims[i];

		PyArrayObject *res =
		    (PyArrayObject *) PyArray_SimpleNewFromData(countdims, dims,
								ret_py_type,
								data);

		// array does not own data, shape can go
    PyArray_CLEARFLAGS(res,NPY_ARRAY_OWNDATA);
		free(dims);

		free(recvmsg);

		return (PyObject *) res;
	} else {
		PYTHON_SVIPC_ERROR("status %d\n", status);
	}

}

/*******************************************************************
 * module static method
 *******************************************************************/

static struct PyMethodDef python_svipc_methods[] = {
	{"setaffinity", (PyCFunction) python_svipc_misc_setaffinity,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_misc_setaffinity_doc},
	{"ftok", (PyCFunction) python_svipc_misc_ftok,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_misc_ftok_doc},
	{"nprocs", (PyCFunction) python_svipc_misc_nprocs, METH_NOARGS,
	 python_svipc_misc_nprocs_doc},

	{"shm_info", (PyCFunction) python_svipc_shm_info,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_shm_info_doc},
	{"shm_init", (PyCFunction) python_svipc_shm_init,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_shm_init_doc},
	{"shm_write", (PyCFunction) python_svipc_shm_write,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_shm_write_doc},
	{"shm_read", (PyCFunction) python_svipc_shm_read,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_shm_read_doc},
	{"shm_free", (PyCFunction) python_svipc_shm_free,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_shm_free_doc},
	{"shm_cleanup", (PyCFunction) python_svipc_shm_cleanup,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_shm_cleanup_doc},

	{"sem_info", (PyCFunction) python_svipc_sem_info,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_sem_info_doc},
	{"sem_init", (PyCFunction) python_svipc_sem_init,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_sem_init_doc},
	{"sem_cleanup", (PyCFunction) python_svipc_sem_cleanup,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_sem_cleanup_doc},
	{"sem_take", (PyCFunction) python_svipc_semtake,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_sem_take_doc},
	{"sem_give", (PyCFunction) python_svipc_semgive,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_sem_give_doc},

	{"msq_info", (PyCFunction) python_svipc_msq_info,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_msq_info_doc},
	{"msq_init", (PyCFunction) python_svipc_msq_init,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_msq_init_doc},
	{"msq_cleanup", (PyCFunction) python_svipc_msq_cleanup,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_msq_cleanup_doc},
	{"msq_snd", (PyCFunction) python_svipc_msqsnd,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_msq_snd_doc},
	{"msq_rcv", (PyCFunction) python_svipc_msqrcv,
	 METH_VARARGS | METH_KEYWORDS, python_svipc_msq_rcv_doc},

	{NULL}			/* sentinel */
};

/*******************************************************************
 * NAME
 *  initsvipc - module initialization
 *
 * DESCRIPTION
 *	Initialize svipc module
 *
 * WARNING
 *	this procedure must be called <modulename>init
 *
 * PYTHON API:
 *	import svipc
 *
 * PERSONNEL:
 *  	Matthieu Bec, Gemini Software Group -  mbec@gemini.edu
 * 	
 * HISTORY:
 *  	22/02/10 - created
 *
 *******************************************************************/

PyDoc_STRVAR(python_svipc_doc, "SysV IPC for Python.\n\
\n\
A module that encapsulates SysV IPC.\n\
");

#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_svipc(void)
#else
PyMODINIT_FUNC initsvipc(void)
#endif
{

	// initialize Python with thread support
	Py_Initialize();

	// initialize numpy
	import_array();

	/* Create the module and associated method */
#if PY_MAJOR_VERSION >= 3
	static struct PyModuleDef svipcdef = {
	  PyModuleDef_HEAD_INIT,
	  "svipc",     /* m_name */
	  python_svipc_doc,  /* m_doc */
	  -1,                  /* m_size */
	  python_svipc_methods,    /* m_methods */
	  NULL,                /* m_reload */
	  NULL,                /* m_traverse */
	  NULL,                /* m_clear */
	  NULL,                /* m_free */
	};
	python_svipc_module = PyModule_Create(&svipcdef);
#else
	python_svipc_module = Py_InitModule3("svipc", python_svipc_methods,
					     python_svipc_doc);
#endif

	if (python_svipc_module == NULL)
#if PY_MAJOR_VERSION >= 3
	     return NULL;
#else
	     return;
#endif

	/* Add symbolic constants to the module */
	PyModule_AddStringConstant(python_svipc_module, "version",
				   PYTHON_SVIPC_VERSION);

	/* define module generic error */
	python_svipc_error = PyErr_NewException("svipc.error", NULL, NULL);
	PyModule_AddObject(python_svipc_module, "error", python_svipc_error);

	/* Check for errors */
	if (PyErr_Occurred())
		Py_FatalError("can't initialize module svipc");

	// invoke something when the interpreter dies
	// Py_AtExit(python_svipc_cleanup);
#if PY_MAJOR_VERSION >= 3
	return python_svipc_module;
#endif

}
