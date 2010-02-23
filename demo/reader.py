#! /usr/bin/env python

import svipc

a = svipc.read(0xbadcafe, 'momo')
print a.sum()
   