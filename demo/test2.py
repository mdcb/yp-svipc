#! /usr/bin/env python

import svipc
import numpy

svipc.shm_init(0xbadcafe,3)
svipc.shm_info(0xbadcafe, 1)

a = (numpy.arange(4)+1).reshape(2,2)
svipc.shm_write(0xbadcafe, 'momo',a)
svipc.shm_info(0xbadcafe, 1)
#svipc.shm_cleanup(0xbadcafe)
#a = svipc.shm_read(0xbadcafe, 'momo')
