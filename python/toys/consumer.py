#! /usr/bin/env python

import svipc

num = 0
while True:
   a = svipc.read(0xbadcafe, 'momo', 1)
   num +=1
   print num, a.sum()
   