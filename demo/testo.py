#! /usr/bin/env python

#import svipc

#print svipc.version

# svipc.init(0xbadcafe,10)
#svipc.info(0xbadcafe,2)
#res = svipc.read(0xbadcafe,'momo')
#print res
#print res.dtype
#print res.shape
# svipc.cleanup(0xbadcafe)

#svipc.write(0xbadcafe, 'momo',[1,2,3,4])
#svipc.write(0xbadcafe, 'momo',[[1,2,3],[5,6,7]])
#
#a = numpy.array([[1,2],[3,4],[5,6],[7,8]],dtype=numpy.int8)
#svipc.write(0xbadcafe, 'momo',a)
#
#a = numpy.array([[1,2],[3,4],[5,6],[7,8]],dtype=numpy.int16)
#svipc.write(0xbadcafe, 'momo',a)
#
#a = numpy.array([[1,2],[3,4],[5,6],[7,8]],dtype=numpy.int32)
#svipc.write(0xbadcafe, 'momo',a)
#
#a = numpy.array([[1,2],[3,4],[5,6],[7,8]],dtype=numpy.float32)
#svipc.write(0xbadcafe, 'momo',a)
#
#a = numpy.array([[1,2],[3,4],[5,6],[7,8]],dtype=numpy.float64)
#svipc.write(0xbadcafe, 'momo',a)

import svipc
import numpy
#svipc.cleanup(0xbadcafe)
#svipc.init(0xbadcafe,3)
svipc.init(0xbadcafe,3)
#a = numpy.array([[-1,-2],[-3,-4]],dtype=numpy.float64)
#svipc.write(0xbadcafe, 'momo',a)
svipc.info(0xbadcafe, 10)
svipc.cleanup(0xbadcafe)

