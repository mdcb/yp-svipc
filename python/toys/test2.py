#! /usr/bin/env python

import svipc
import numpy

svipc.init(0xbadcafe,3)
svipc.info(0xbadcafe, 1)

a = (numpy.arange(4)+1).reshape(2,2)
svipc.write(0xbadcafe, 'momo',a)
svipc.info(0xbadcafe, 1)
#svipc.cleanup(0xbadcafe)
#a = svipc.read(0xbadcafe, 'momo')
