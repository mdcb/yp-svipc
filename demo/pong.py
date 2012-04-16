#! /usr/bin/env python

import svipc
import sys
import time



# retreive ids
my_shmid,my_semid,my_msqid = [int(a) for a in sys.argv[1:]]

#let yorick wait .. for fun
sys.stderr.write('P: sleeping 1s.\n')
time.sleep(1)

# tell it to go
sys.stderr.write('P: yorick may go.\n')
svipc.sem_give(my_semid,id=0)

# play 1
for i in range(3):
   time.sleep(1)
   sys.stderr.write('P: sem give %d.\n' % i)
   svipc.sem_give(key=my_semid,id=0)


# play 2

sys.stderr.write('P: wait synch from yorick\n')

svipc.sem_take(my_semid,1)

sys.stderr.write('P: going now\n')

for i in range(3):
   msg = svipc.shm_read(my_shmid,id="pingpong",subscribe=-1.0)
   time.sleep(1)
   msg[i]=0
   svipc.shm_write(my_shmid,"pingpong",data=msg,publish=1)
  
# play 3

sys.stderr.write('P: wait synch from yorick\n')
svipc.sem_take(my_semid,1)
sys.stderr.write('P: going now\n')


for i in range(3):
   sys.stderr.write('P: rcv\n')
   msg = svipc.msq_rcv(my_msqid,mtype=1234)
   msg[i]=0
   sys.stderr.write('P: snd\n')
   svipc.msq_snd(my_msqid,mtype=5678,data=msg)

# tell yorick we're done
svipc.sem_give(my_semid,id=0)

sys.stderr.write('P: exiting\n')
exit()

