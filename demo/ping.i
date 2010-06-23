#include "svipc.i"

// create ids
my_shmid = 0x80800000 | getpid();
my_semid = 0xdcb00000 | getpid();
my_msqid = 0x10100000 | getpid();

// create a pool of shared memory
shm_init,my_shmid,slots=1;
sem_init,my_semid,nums=1;
msq_init,my_msqid;

// spawn pong.py
func on_py_stdout(msg) {
   write,"from python:",msg;
}

pyfd = spawn(["python","pong.py", 
   swrite(format="%d",my_shmid),
   swrite(format="%d",my_semid),
   swrite(format="%d",my_msqid)
   ], on_py_stdout);

write,"wait to synch with python ..."
sem_take,my_semid,0;

// play 1

write,"--- sem 3 times";

for (i=0;i<3;i++) {
   i;
   sem_take,my_semid,0;
}

// play 2

write,"--- shm 3 times";

// create the shm and tell python play2 is on
msg = [1,2,3,4]
shm_write,my_shmid,"pingpong",&msg;

sem_give,my_semid,0;
pause,500; // allow python to get into its subscribe before we send our publish

for (i=0;i<3;i++) {
   write,"sending";
   msg;
   shm_write,my_shmid,"pingpong",&msg,publish=1;
   
   reply = shm_read(my_shmid,"pingpong",subscribe=-1);
   write,"received";
   reply;
}

// play 3

write,"--- msq 3 times";

for (i=0;i<3;i++) {
   write,"sending";
   msg;
   msq_snd,my_msqid,1234,&msg;
   reply = msq_rcv(my_msqid,5678);
   write,"received";
   reply;
}





// cleanup
shm_cleanup, my_shmid;
sem_cleanup, my_semid;
msq_cleanup, my_msqid;

// bye
quit;