#!/usr/bin/env python

from numpy.distutils.core import setup, Extension
from os import uname, environ as env

version = '0.12'                  # TODO yorick/svipc.i:SVIPC_VERSION

undef_macros=[
   # 'PYTHON_SVIPC_NODEBUG',      # disable at compilation time.
   ]

define_macros=[
   ('PYTHON_SVIPC_VERSION',          '\\\"%s\\\"' % version),
   ('SVIPC_NOSEGFUNC',               None),
   ]

platform=uname()[0]
if platform != 'Linux':
   define_macros.append(('SVIPC_HACKS', None))

extra_compile_args=[
   #'-g3',
   #'-ggdb3'
]

setup(name='python-svipc',
      version=version,
      description='System V IPC for Python',
      long_description='A python plugin to System V IPC.',
      author='Matthieu D.C. Bec',
      url='https://github.com/mdcb',
      author_email='mdcb808@gmail.com',
      platforms=platform,
      license='GPL',
      ext_modules=[
         Extension(
            name='svipc',
            sources = [
               'common/svipc_misc.c',
               'common/svipc_shm.c',
               'common/svipc_sem.c',
               'common/svipc_msq.c',
               'python/svipc_module.c',
              ],
            include_dirs=['common'],
            define_macros=define_macros,
            undef_macros=undef_macros,
            extra_compile_args = extra_compile_args,
            )
         ],
      )
     
