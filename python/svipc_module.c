// System-V IPC plugin for Python
// Matthieu D.C. Bec 23/04/2010
// GNU Public License (GPLv3) applies - see www.gnu.org

#include "Python.h"
#include <numpy/arrayobject.h>

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
 * ftok
 *******************************************************************/
PyDoc_STRVAR(python_svipc_misc_ftok_doc, "ftok(path, proj=)\n\
Convert a pathname and a project identifier to a System V IPC key\n\
");

PyObject *python_svipc_misc_ftok(PyObject * self, PyObject * args)
{

   char *path;
   int proj = 0;

   if (!PyArg_ParseTuple(args, "s|i", &path, &proj))
      PYTHON_SVIPC_USAGE("ftok(path, proj=)");

   long key = svipc_ftok(path, proj);

   return PyInt_FromLong(key);
}

/*******************************************************************
 * nprocs
 *******************************************************************/
PyDoc_STRVAR(python_svipc_misc_nprocs_doc, "nprocs()\n\
Returns the number of processors currently online (available).\n\
");

PyObject *python_svipc_misc_nprocs(PyObject * self, PyObject * args)
{
   return PyInt_FromLong(svipc_nprocs());
}

/*******************************************************************
 * shm info
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_info_doc, "shm_info(key, details=)\n\
Print a report on shared memory pool identified by 'key'.\n\
'details' controls the level of information printed out.\n\
");

PyObject *python_svipc_shm_info(PyObject * self, PyObject * args)
{
   int key, details = 0;

   if (!PyArg_ParseTuple(args, "i|i", &key, &details))
      PYTHON_SVIPC_USAGE("shm_info(key, details=)");

   int status = svipc_shm_info(key, details);

   return PyInt_FromLong(status);
}

/*******************************************************************
 * shm init
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_init_doc, "shm_init(key, slots=)\n\
Initialize a pool of shared memory identified by 'key' containing\n\
'slots' segments of initially free Ids\n\
");

PyObject *python_svipc_shm_init(PyObject * self, PyObject * args)
{
   int key, numslots = -1;

   if (!PyArg_ParseTuple(args, "i|i", &key, &numslots))
      PYTHON_SVIPC_USAGE("shm_init(key, slots=)");

   int status = svipc_shm_init(key, numslots);

   return PyInt_FromLong(status);
}

/*******************************************************************
 * shm write
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_write_doc, "shm_write(key,id,a,publish=0)\n\
Write the content of the variable referenced by a in\n\
the slot identified by 'id' from the shared memory pool\n\
identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency for external readers.\n\
");

PyObject *python_svipc_shm_write(PyObject * self, PyObject * args)
{

   int key;
   char *id;
   PyObject *a;
   int publish = 0;

   slot_array arr;

   if (!PyArg_ParseTuple(args, "isO|i", &key, &id, &a, &publish))
      PYTHON_SVIPC_USAGE("shm_write(key, id,a)");

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
   arr.number = (int *) malloc(arr.countdims * sizeof(*arr.number));
   memcpy(arr.number, PyArray_DIMS(inp_array), arr.countdims * sizeof(*arr.number));

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
Read the content of the slot identified by 'id' from the\n\
shared memory pool identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency with external writers.\n\
");

PyObject *python_svipc_shm_read(PyObject * self, PyObject * args)
{
   int key;
   char *id;
   slot_array arr;
   float subscribe = 0.0;

   if (!PyArg_ParseTuple(args, "is|f", &key, &id, &subscribe))
      PYTHON_SVIPC_USAGE("shm_read(key, id)");

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

      PyArrayObject *res = (PyArrayObject *) PyArray_SimpleNewFromData(arr.countdims, arr.number, ret_py_type, arr.data);

      // array owns data, shape can go
      PyArray_FLAGS(res) |= NPY_OWNDATA;

      return (PyObject *) res;
   } else {
      PYTHON_SVIPC_ERROR("status %d\n", status);
   }
}

/*******************************************************************
 * shm free
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_free_doc, "shm_free(key,id)\n\
Release the slot identified by 'id' from the\n\
shared memory pool identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency with external readers and writers.\n\
");

PyObject *python_svipc_shm_free(PyObject * self, PyObject * args)
{

   int key;
   char *id;

   if (!PyArg_ParseTuple(args, "is", &key, &id))
      PYTHON_SVIPC_USAGE("shm_free(key, id)");

   int status = svipc_shm_free(key, id);

   return PyInt_FromLong(status);

}

/*******************************************************************
 * shm cleanup
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_cleanup_doc, "shm_cleanup(key)\n\
Release all the slots from the shared memory pool\n\
identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency with external readers and writers.\n\
");

PyObject *python_svipc_shm_cleanup(PyObject * self, PyObject * args)
{

   int key;

   if (!PyArg_ParseTuple(args, "i", &key))
      PYTHON_SVIPC_USAGE("shm_cleanup(key)");

   int status = svipc_shm_cleanup(key);

   return PyInt_FromLong(status);

}

/*******************************************************************
 * sem info
 *******************************************************************/
PyDoc_STRVAR(python_svipc_sem_info_doc, "sem_info(key, details=)\n\
Print a report on semaphore pool identified by 'key'.\n\
'details' controls the level of information printed out.\n\
");

PyObject *python_svipc_sem_info(PyObject * self, PyObject * args)
{
   int key;
   int details = 0;

   if (!PyArg_ParseTuple(args, "i|i", &key, &details))
      PYTHON_SVIPC_USAGE("sem_info(key, details=)");

   int status = svipc_sem_info(key, details);

   return PyInt_FromLong(status);
}

/*******************************************************************
 * sem init
 *******************************************************************/
PyDoc_STRVAR(python_svipc_sem_init_doc, "sem_init(key, nums=)\n\
Initialize a pool of semaphores identified by 'key' containing\n\
'nums' initially taken (locked) semaphores.\n\
");

PyObject *python_svipc_sem_init(PyObject * self, PyObject * args)
{
   int key, nums = -1;

   if (!PyArg_ParseTuple(args, "i|i", &key, &nums))
      PYTHON_SVIPC_USAGE("sem_init(key, nums=)");

   int status = svipc_sem_init(key, nums);

   return PyInt_FromLong(status);
}

/*******************************************************************
 * sem cleanup
 *******************************************************************/
PyDoc_STRVAR(python_svipc_sem_cleanup_doc, "sem_cleanup(key)\n\
Release the semaphore pool identified by 'key'.\n\
");

PyObject *python_svipc_sem_cleanup(PyObject * self, PyObject * args)
{

   int key;

   if (!PyArg_ParseTuple(args, "i", &key))
      PYTHON_SVIPC_USAGE("sem_cleanup(key)");

   int status = svipc_sem_cleanup(key);

   return PyInt_FromLong(status);

}

/*******************************************************************
 * sem take
 *******************************************************************/
PyDoc_STRVAR(python_svipc_sem_take_doc, "sem_take(key,id,count=1,wait=-1)\n\
Take the semaphore id from the pool identified by 'key'.\n\
Waiting up to 'wait' seconds.\n\
");

PyObject *python_svipc_semtake(PyObject * self, PyObject * args)
{

   int key, id, count;
   float wait = -1;
   count = 1;
   if (!PyArg_ParseTuple(args, "ii|if", &key, &id, &count, &wait))
      PYTHON_SVIPC_USAGE("sem_take(key,id,wait=-1)");

   int status = svipc_semtake(key, id, count, wait);

   return PyInt_FromLong(status);

}

/*******************************************************************
 * sem give
 *******************************************************************/
PyDoc_STRVAR(python_svipc_sem_give_doc, "sem_give(key,id,count=1)\n\
Give the semaphore id from the pool identified by 'key'.\n\
");

PyObject *python_svipc_semgive(PyObject * self, PyObject * args)
{

   int key, id, count;

   count = 1;
   if (!PyArg_ParseTuple(args, "ii|i", &key, &id, &count))
      PYTHON_SVIPC_USAGE("sem_give(key,id)");

   int status = svipc_semgive(key, id, count);

   return PyInt_FromLong(status);

}

/*******************************************************************
 * msq info
 *******************************************************************/
PyDoc_STRVAR(python_svipc_msq_info_doc, "msq_info(key, details=)\n\
Print a report on message queue identified by 'key'.\n\
'details' controls the level of information printed out.\n\
");

PyObject *python_svipc_msq_info(PyObject * self, PyObject * args)
{
   int key;
   int details = 0;

   if (!PyArg_ParseTuple(args, "i|i", &key, &details))
      PYTHON_SVIPC_USAGE("msq_info(key, details=)");

   int status = svipc_msq_info(key, details);

   return PyInt_FromLong(status);
}

/*******************************************************************
 * msq init
 *******************************************************************/
PyDoc_STRVAR(python_svipc_msq_init_doc, "msq_init(key)\n\
Initialize a message queue identified by 'key'.\n\
");

PyObject *python_svipc_msq_init(PyObject * self, PyObject * args)
{
   int key;

   if (!PyArg_ParseTuple(args, "i", &key))
      PYTHON_SVIPC_USAGE("msq_init(key)");

   int status = svipc_msq_init(key);

   return PyInt_FromLong(status);
}

/*******************************************************************
 * msq cleanup
 *******************************************************************/
PyDoc_STRVAR(python_svipc_msq_cleanup_doc, "msq_cleanup(key)\n\
Release the message queue identified by 'key'.\n\
");

PyObject *python_svipc_msq_cleanup(PyObject * self, PyObject * args)
{

   int key;

   if (!PyArg_ParseTuple(args, "i", &key))
      PYTHON_SVIPC_USAGE("msq_cleanup(key)");

   int status = svipc_msq_cleanup(key);

   return PyInt_FromLong(status);

}

/*******************************************************************
 * msq snd
 *******************************************************************/
PyDoc_STRVAR(python_svipc_msq_snd_doc, "msq_snd(key,type,a,nowait=0)\n\
Sends a message to queue identified by 'key'.\n\
");

PyObject *python_svipc_msqsnd(PyObject * self, PyObject * args)
{
   int key;
   int type;
   PyObject *a;
   int nowait = 0;

   if (!PyArg_ParseTuple(args, "iiO|i", &key, &type, &a, &nowait))
      PYTHON_SVIPC_USAGE("msq_snd(key,type,a,nowait=0)");

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
   
   size_t msgsz = sizeof(typeid)+sizeof(countdims)+countdims*sizeof(countdims)+totalnumber*sizeoftype;
   struct svipc_msgbuf *sendmsg = malloc(sizeof(struct svipc_msgbuf) + msgsz);
   
   sendmsg->mtype = type;
   
   int *msgp_pint = (int*)sendmsg->mtext;
   *msgp_pint++ = typeid;
   *msgp_pint++ = countdims;
   int i;
   for (i=0;i<countdims;i++) {
      *msgp_pint++ = *((int*)PyArray_DIMS(inp_array)+i);
   }
   memcpy(msgp_pint,PyArray_DATA(inp_array),totalnumber*sizeoftype);
   
   
   int status = svipc_msq_snd(key, sendmsg, msgsz, nowait);

   free(sendmsg);
   Py_DECREF(inp_array);

   return PyInt_FromLong(status);
}

/*******************************************************************
 * msq rcv
 *******************************************************************/
PyDoc_STRVAR(python_svipc_msq_rcv_doc, "msq_rcv(key,type,nowait=0)\n\
Receive a message to queue identified by 'key'.\n\
");

PyObject *python_svipc_msqrcv(PyObject * self, PyObject * args)
{

   int key;
   int type;
   int nowait = 0;
   
   if (!PyArg_ParseTuple(args, "ii|i", &key, &type, &nowait))
      PYTHON_SVIPC_USAGE("msq_rcv(key,type,nowait=0)");

   int *msgp_pint;
   struct svipc_msgbuf *recvmsg;
   
   int status = svipc_msq_rcv(key, type, &recvmsg, nowait);
   

   if (status == 0) {
      msgp_pint = (int*)recvmsg->mtext;
      int typeid = *msgp_pint++;
      int countdims = *msgp_pint++;
      int *dims = msgp_pint;
      int *data = dims+countdims;
      
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

      PyArrayObject *res = (PyArrayObject *) PyArray_SimpleNewFromData(countdims, dims, ret_py_type, data);

      // array does not own data
      PyArray_FLAGS(res) &= ~NPY_OWNDATA;

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
   {"ftok", (PyCFunction) python_svipc_misc_ftok, METH_VARARGS, python_svipc_misc_ftok_doc},
   {"nprocs", (PyCFunction) python_svipc_misc_nprocs, METH_VARARGS, python_svipc_misc_nprocs_doc},
   
   {"shm_info", (PyCFunction) python_svipc_shm_info, METH_VARARGS, python_svipc_shm_info_doc},
   {"shm_init", (PyCFunction) python_svipc_shm_init, METH_VARARGS, python_svipc_shm_init_doc},
   {"shm_write", (PyCFunction) python_svipc_shm_write, METH_VARARGS, python_svipc_shm_write_doc},
   {"shm_read", (PyCFunction) python_svipc_shm_read, METH_VARARGS, python_svipc_shm_read_doc},
   {"shm_free", (PyCFunction) python_svipc_shm_free, METH_VARARGS, python_svipc_shm_free_doc},
   {"shm_cleanup", (PyCFunction) python_svipc_shm_cleanup, METH_VARARGS, python_svipc_shm_cleanup_doc},
   
   {"sem_info", (PyCFunction) python_svipc_sem_info, METH_VARARGS, python_svipc_sem_info_doc},
   {"sem_init", (PyCFunction) python_svipc_sem_init, METH_VARARGS, python_svipc_sem_init_doc},
   {"sem_cleanup", (PyCFunction) python_svipc_sem_cleanup, METH_VARARGS, python_svipc_sem_cleanup_doc},
   {"sem_take", (PyCFunction) python_svipc_semtake, METH_VARARGS, python_svipc_sem_take_doc},
   {"sem_give", (PyCFunction) python_svipc_semgive, METH_VARARGS, python_svipc_sem_give_doc},
   
   {"msq_info", (PyCFunction) python_svipc_msq_info, METH_VARARGS, python_svipc_msq_info_doc},
   {"msq_init", (PyCFunction) python_svipc_msq_init, METH_VARARGS, python_svipc_msq_init_doc},
   {"msq_cleanup", (PyCFunction) python_svipc_msq_cleanup, METH_VARARGS, python_svipc_msq_cleanup_doc},
   {"msq_snd", (PyCFunction) python_svipc_msqsnd, METH_VARARGS, python_svipc_msq_snd_doc},
   {"msq_rcv", (PyCFunction) python_svipc_msqrcv, METH_VARARGS, python_svipc_msq_rcv_doc},
   
   {NULL}                       /* sentinel */
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

PyMODINIT_FUNC initsvipc()
{

   // initialize Python with thread support
   Py_Initialize();

   // initialize numpy
   import_array();

   /* Create the module and associated method */
   if ((python_svipc_module = Py_InitModule3("svipc", python_svipc_methods, python_svipc_doc)) == NULL)
      return;

   /* Add symbolic constants to the module */
   PyModule_AddStringConstant(python_svipc_module, "version", PYTHON_SVIPC_VERSION);

   /* define module generic error */
   python_svipc_error = PyErr_NewException("svipc.error", NULL, NULL);
   PyModule_AddObject(python_svipc_module, "error", python_svipc_error);

   /* Check for errors */
   if (PyErr_Occurred())
      Py_FatalError("can't initialize module svipc");

   // invoke something when the interpreter dies
   // Py_AtExit(python_svipc_cleanup);

}
