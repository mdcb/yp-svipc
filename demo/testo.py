#! /usr/bin/env python

#import svipc

#print svipc.version

# svipc.shm_init(0xbadcafe,10)
#svipc.shm_info(0xbadcafe,2)
#res = svipc.shm_read(0xbadcafe,'momo')
#print res
#print res.dtype
#print res.shape
# svipc.shm_cleanup(0xbadcafe)

#svipc.shm_write(0xbadcafe, 'momo',[1,2,3,4])
#svipc.shm_write(0xbadcafe, 'momo',[[1,2,3],[5,6,7]])
#
#a = numpy.array([[1,2],[3,4],[5,6],[7,8]],dtype=numpy.int8)
#svipc.shm_write(0xbadcafe, 'momo',a)
#
#a = numpy.array([[1,2],[3,4],[5,6],[7,8]],dtype=numpy.int16)
#svipc.shm_write(0xbadcafe, 'momo',a)
#
#a = numpy.array([[1,2],[3,4],[5,6],[7,8]],dtype=numpy.int32)
#svipc.shm_write(0xbadcafe, 'momo',a)
#
#a = numpy.array([[1,2],[3,4],[5,6],[7,8]],dtype=numpy.float32)
#svipc.shm_write(0xbadcafe, 'momo',a)
#
#a = numpy.array([[1,2],[3,4],[5,6],[7,8]],dtype=numpy.float64)
#svipc.write(0xbadcafe, 'momo',a)

import svipc
import numpy
#svipc.shm_cleanup(0xbadcafe)
#svipc.shm_init(0xbadcafe,3)
svipc.shm_init(0xbadcafe,3)
#a = numpy.array([[-1,-2],[-3,-4]],dtype=numpy.float64)
#svipc.shm_write(0xbadcafe, 'momo',a)
svipc.shm_info(0xbadcafe, 10)
svipc.shm_cleanup(0xbadcafe)

