#!/usr/bin/env python
#
#  Copyright (C) 2011-2012  Matthieu Bec.
#
#  This file is part of yp-svipc.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

from numpy.distutils.core import setup, Extension
from os import uname, environ as env

version = '0.16'                  # TODO yorick/svipc.i:SVIPC_VERSION

undef_macros=[
   # 'PYTHON_SVIPC_NODEBUG',      # disable at compilation time.
   ]

define_macros=[
   ('PYTHON_SVIPC_VERSION',          '\\\"%s\\\"' % version),
   ('SVIPC_NOSEGFUNC',               None),
   ]

platform=uname()[0]

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
     
