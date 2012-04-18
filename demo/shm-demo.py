#! /usr/bin/env python

#
# cpu hog - demo used to check we're not leaking
#

from os import getpid, fork
from svipc import shm_init, shm_read, shm_write, shm_cleanup
from svipc import sem_init, sem_take, sem_give, sem_cleanup
import numpy
import time

num_= 10000000

n = 32

stoopid=numpy.ones(n)
stoopid[n-1]=n
data = numpy.arange(n).reshape(stoopid)
#print data.shape


# create id
svid0 = 0x7dcb0000 | getpid()
svid1 = svid0+1

# create a pool with one shmaphore
shm_init(svid0,slots=1)
sem_init(svid1,nums=1)

if fork():
  for i in range(num_):
    shm_write(svid0,'slot0',data,publish=0) # shm id's are string ... doh
    sem_take(svid1,0)
else:
  time.sleep(1)
  for i in range(num_):
    d=shm_read(svid0,'slot0',0)
    sem_give(svid1,0)
    ### print d
  exit()

sem_cleanup(svid1)
shm_cleanup(svid0)
