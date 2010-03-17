#! /usr/bin/env python

import svipc

//blocking sync
num = 0
while True:
   a = svipc.shm_read(0xbadcafe, 'momo', subscribe=-1.0)
   num +=1
   print num, a.sum()


// poll every 1sec
num = 0
while True:
   a = svipc.shm_read(0xbadcafe, 'momo', subscribe=1.0)
   num +=1
   print num, a.sum()

   