#! /usr/bin/env python

import svipc
import numpy

svipc.init(0xbadcafe,3)
svipc.info(0xbadcafe, 1)

a = numpy.zeros([1024,1024],dtype=numpy.float64)
print 'write #1', svipc.write(0xbadcafe, 'momo',a)
a = numpy.ones([1024,1024],dtype=numpy.float64)
print 'write #2', svipc.write(0xbadcafe, 'momo',a)

svipc.cleanup(0xbadcafe)

#a = svipc.read(0xbadcafe, 'momo')
