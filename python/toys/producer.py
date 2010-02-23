#! /usr/bin/env python

import svipc
import numpy

svipc.init(0xbadcafe,3)
svipc.info(0xbadcafe, 1)

a = numpy.zeros([1024,1024],dtype=numpy.float64)
#a = numpy.ones([1024,1024],dtype=numpy.float64)
a = (numpy.arange(4)+1).reshape(2,2)
svipc.write(0xbadcafe, 'momo',a)

svipc.info(0xbadcafe, 1)

#svipc.publish(0xbadcafe, 'momo',a)

import time

num = 0
while True:
   a = numpy.array(numpy.random.rand(1024*1024),dtype=numpy.float64).reshape(1024,1024)
   svipc.write(0xbadcafe, 'momo',a, 1)
   num += 1
   print num, a.sum()
   #time.sleep(0.005)
   #time.sleep(0.01)

#svipc.cleanup(0xbadcafe)
