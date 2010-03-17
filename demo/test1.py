#! /usr/bin/env python

import svipc
import numpy

svipc.shm_init(0xbadcafe,3)
svipc.shm_info(0xbadcafe, 1)

a = numpy.zeros([1024,1024],dtype=numpy.float64)
print 'write #1', svipc.shm_write(0xbadcafe, 'momo',a)
a = numpy.ones([1024,1024],dtype=numpy.float64)
print 'write #2', svipc.shm_write(0xbadcafe, 'momo',a)

svipc.shm_cleanup(0xbadcafe)

#a = svipc.shm_read(0xbadcafe, 'momo')
