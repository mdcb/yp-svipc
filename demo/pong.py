#! /usr/bin/env python

import svipc
import sys
import time



# retreive ids
my_shmid,my_semid,my_msqid = [long(a) for a in sys.argv[1:]]

# wait one sec

#let yorick wait ..
time.sleep(1)

# tell it to go
svipc.sem_give(my_semid,id=0)

# play 1

for i in range(3):
   time.sleep(1)
   svipc.sem_give(key=my_semid,id=0)

# play 2

svipc.sem_take(my_semid,0)

for i in range(3):
   msg = svipc.shm_read(my_shmid,id="pingpong",subscribe=-1.0)
   time.sleep(1)
   msg[i]=0
   svipc.shm_write(my_shmid,"pingpong",data=msg,publish=1)
  
# play 3

for i in range(3):
   msg = svipc.msq_rcv(my_msqid,mtype=1234)
   msg[i]=0
   svipc.msq_snd(my_msqid,mtype=5678,data=msg)




