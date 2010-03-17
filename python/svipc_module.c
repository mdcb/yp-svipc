#include "Python.h"
#include <numpy/arrayobject.h>

#include <stdio.h>
#include <string.h>

#include "svipc_shm.h"

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
PyDoc_STRVAR(python_svipc_shm_ftok_doc,
"ftok(path, proj=)\n\
Convert a pathname and a project identifier to a System V IPC key\n\
");

PyObject * python_svipc_shm_ftok (
	PyObject *self,
	PyObject *args
	) {

	char *path;
   int proj=0;
   
   if (!PyArg_ParseTuple(args, "s|i",&path,&proj))
		PYTHON_SVIPC_USAGE("ftok(path, proj=)");
   
   long key = svipc_ftok(path, proj);
   
	return PyInt_FromLong(key);
	}

/*******************************************************************
 * info
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_info_doc,
"info(key, details=)\n\
Print a report on shared memory pool identified by 'key'.\n\
'details' controls the level of information printed out.\n\
");

PyObject * python_svipc_shm_info (
	PyObject *self,
	PyObject *args
	) {
	long key,details=0;
   
   if (!PyArg_ParseTuple(args, "l|l",&key,&details))
		PYTHON_SVIPC_USAGE("info(key, details=)");
   
   int status = svipc_shm_info(key, details);
   
   return PyInt_FromLong(status);
   }

/*******************************************************************
 * init
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_init_doc,
"init(key, slots=)\n\
Initialize a pool of shared memory identified by 'key' containing\n\
'slots' segments of initially free Ids\n\
");

PyObject * python_svipc_shm_init (
	PyObject *self,
	PyObject *args
	) {
	long key, numslots=-1;
	
	
	if (!PyArg_ParseTuple(args, "l|l",&key,&numslots))
		PYTHON_SVIPC_USAGE("init(key, slots=)");
      
   int status = svipc_shm_init(key, numslots);
   
   return PyInt_FromLong(status);
	}

/*******************************************************************
 * write
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_write_doc,
"write(key,id,a,publish=0)\n\
Write the content of the variable referenced by a in\n\
the slot identified by 'id' from the shared memory pool\n\
identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency for external readers.\n\
");

PyObject * python_svipc_shm_write (
	PyObject *self,
	PyObject *args
	) {

	long key;
   char *id;
   PyObject *a;
   int publish=0;
   
   slot_array arr;
	
	if (!PyArg_ParseTuple(args, "lsO|i",&key,&id,&a,&publish))
		PYTHON_SVIPC_USAGE("write(key, id,a)");
	
   PyArrayObject *inp_array = (PyArrayObject *) PyArray_FROM_O(a);
	
   if (PyArray_TYPE(inp_array) == NPY_BYTE) arr.typeid = SVIPC_CHAR;
   else if (PyArray_TYPE(inp_array) == NPY_SHORT) arr.typeid = SVIPC_SHORT;
   else if (PyArray_TYPE(inp_array) == NPY_INT) arr.typeid = SVIPC_INT;
   else if (PyArray_TYPE(inp_array) == NPY_LONG) arr.typeid = SVIPC_LONG;
   else if (PyArray_TYPE(inp_array) == NPY_FLOAT) arr.typeid = SVIPC_FLOAT;
   else if (PyArray_TYPE(inp_array) == NPY_DOUBLE) arr.typeid = SVIPC_DOUBLE;
   else {
      PYTHON_SVIPC_ERROR("type not supported");
   }
   
   arr.countdims = PyArray_NDIM(inp_array);
   arr.number = (int*) malloc(arr.countdims * sizeof(*arr.number));
   memcpy (arr.number, PyArray_DIMS(inp_array), arr.countdims * sizeof(*arr.number));
   
   arr.data = PyArray_DATA(inp_array);
   
   int status = svipc_shm_write(key, id, &arr, publish);
   
   free(arr.number);
   Py_DECREF(inp_array);
   
   return PyInt_FromLong(status);
   
   }

/*******************************************************************
 * read
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_read_doc,
"read(key,id,subscribe=0)\n\
Read the content of the slot identified by 'id' from the\n\
shared memory pool identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency with external writers.\n\
");

PyObject * python_svipc_shm_read (
	PyObject *self,
	PyObject *args
	) {
	long key;
   char *id;
	slot_array arr;
	float subscribe=0.0;
   
	if (!PyArg_ParseTuple(args, "ls|f",&key,&id,&subscribe))
		PYTHON_SVIPC_USAGE("read(key, id)");
   
   memset(&arr,0, sizeof(arr));
   int status = svipc_shm_read(key, id, &arr, subscribe);
   
   if (status==0) {
      enum NPY_TYPES ret_py_type;
      if (arr.typeid==SVIPC_CHAR) ret_py_type = NPY_BYTE;
      else if (arr.typeid==SVIPC_SHORT) ret_py_type = NPY_SHORT;
      else if (arr.typeid==SVIPC_INT) ret_py_type = NPY_INT;
      else if (arr.typeid==SVIPC_LONG) ret_py_type = NPY_LONG;
      else if (arr.typeid==SVIPC_FLOAT) ret_py_type = NPY_FLOAT;
      else if (arr.typeid==SVIPC_DOUBLE) ret_py_type = NPY_DOUBLE;
      else {
         free(arr.number);
         PYTHON_SVIPC_ERROR("type not supported");
      };
         
      PyArrayObject *res = (PyArrayObject *) PyArray_SimpleNewFromData(arr.countdims,arr.number,ret_py_type,arr.data);
      
      // array owns data, shape can go
      PyArray_FLAGS(res) |= NPY_OWNDATA;
      free(arr.number);
      
      return (PyObject *)res;
   } else {
      PYTHON_SVIPC_ERROR("status %d\n", status);
	}
}

/*******************************************************************
 * free
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_free_doc,
"free(key,id)\n\
Release the slot identified by 'id' from the\n\
shared memory pool identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency with external readers and writers.\n\
");

PyObject * python_svipc_shm_free (
	PyObject *self,
	PyObject *args
	) {

	long key;
   char *id;
	
	
	if (!PyArg_ParseTuple(args, "ls",&key,&id))
		PYTHON_SVIPC_USAGE("free(key, id)");
      
   int status = svipc_shm_free(key, id);
   
   return PyInt_FromLong(status);

	}

/*******************************************************************
 * cleanup
 *******************************************************************/
PyDoc_STRVAR(python_svipc_shm_cleanup_doc,
"cleanup(key)\n\
Release all the slots from the shared memory pool\n\
identified by 'key'.\n\
This operation is semaphore protected and guarantees\n\
consistency with external readers and writers.\n\
");

PyObject * python_svipc_shm_cleanup (
	PyObject *self,
	PyObject *args
	) {

	long key;
   
	if (!PyArg_ParseTuple(args, "l",&key))
		PYTHON_SVIPC_USAGE("cleanup(key)");
      
   int status = svipc_shm_cleanup(key);
   
   return PyInt_FromLong(status);

	}


/*******************************************************************
 * module static method
 *******************************************************************/

static struct PyMethodDef python_svipc_methods[] = {
	   {"ftok",		      (PyCFunction)python_svipc_shm_ftok,	      METH_VARARGS, python_svipc_shm_ftok_doc      },
	   {"info",		      (PyCFunction)python_svipc_shm_info,	      METH_VARARGS, python_svipc_shm_info_doc      },
	   {"init",		      (PyCFunction)python_svipc_shm_init,	      METH_VARARGS, python_svipc_shm_init_doc      },
	   {"write",		   (PyCFunction)python_svipc_shm_write,      METH_VARARGS, python_svipc_shm_write_doc     },
	   {"read",		      (PyCFunction)python_svipc_shm_read,	      METH_VARARGS, python_svipc_shm_read_doc      },
	   {"free",		      (PyCFunction)python_svipc_shm_free,	      METH_VARARGS, python_svipc_shm_free_doc      },
	   {"cleanup",		   (PyCFunction)python_svipc_shm_cleanup,    METH_VARARGS, python_svipc_shm_cleanup_doc   },
	   {NULL}	/* sentinel */
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

PyDoc_STRVAR(python_svipc_doc,
"SysV IPC for Python.\n\
\n\
A module that encapsulates SysV IPC.\n\
"
);

PyMODINIT_FUNC initsvipc() {

   // initialize Python with thread support
	Py_Initialize();
   
   // initialize numpy
   import_array();
   
	/* Create the module and associated method */
	if ( (python_svipc_module = Py_InitModule3("svipc",python_svipc_methods,python_svipc_doc)) == NULL)
      return;

 	/* Add symbolic constants to the module */
   PyModule_AddStringConstant(python_svipc_module,"version", PYTHON_SVIPC_VERSION );

   
	/* define module generic error */
	python_svipc_error = PyErr_NewException("svipc.error",NULL,NULL);
	PyModule_AddObject(python_svipc_module, "error" ,python_svipc_error);
	
	/* Check for errors */
	if (PyErr_Occurred())
		Py_FatalError("can't initialize module svipc");
      
   // invoke something when the interpreter dies
   // Py_AtExit(python_svipc_cleanup);
   
	}
