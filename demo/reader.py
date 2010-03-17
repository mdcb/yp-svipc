#! /usr/bin/env python

import svipc

a = svipc.shm_read(0xbadcafe, 'momo')
print a.sum()
   