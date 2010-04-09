#include "svipc.i"

my_semid = 0xdeadbeef;
my_shmid = 0xbadcafe;

// size of the array we'll multiply
// see README for /proc/sys/kernel/shmmax
sz = 1436;
//sz = 1024;
//sz = 512;
//sz = 256;
//sz = 128;

// number of processes
np = nprocs();

// split payload
chunk = sz / np;

//sem_cleanup,my_semid;
//shm_cleanup,my_shmid;

// create 2 semaphores:
// (0) tells the children to start
// (1) syncs the parent when the children complete
sem_init,my_semid,nums=2;

// create np-1 segments of shared memory
// to hold the children results
shm_init,my_shmid,slots=np-1;

// create a random big array
a=float(random(sz,sz)); // create a random big array - of float

id=0

// create np-1 children
for (i=1;i<np;i++) {
   if (fork()!=0) {
   // parent
   continue;
   } else {
   // child
   id=i;
   break;
   }
}

if (id==0) {
   
   // take a baseline compute time measurement
   tic;
   dotprod = a(,+)*a(+,)
   t_baseline = tac();
   write, "--- baseline (1 proc) dotprod:", t_baseline

   // prep the shared memory results
   // fixme - all same size
   onechunk = a(1:chunk,)
   tic;
   for (i=1;i<np;i++) {
      idstr = swrite(format="data%d",i)
      shm_write, my_shmid,idstr,&onechunk;
   }
   t_1 = tac();

   // tell our children it's ok to start
   tic;
   for (i=1;i<np;i++) { sem_give,my_semid,0; } // todo: sem_test rather than sem_take for children
   t_2 = tac();

   // compute our share
   tic;
   zzz = a(1+id*chunk:(id+1)*chunk,)(,+)*a(+,);
   t_3 = tac();

   // wait for all the childrens
   tic;
   for (i=1;i<np;i++) { sem_take,my_semid,1; }
   t_4 = tac();

   tic;
   // check the ops from child (we know ours is good..)
   for (i=1;i<np;i++) {
      idstr = swrite(format="data%d",i);
      xxx = shm_read(my_shmid,idstr);
      if (dimsof(xxx)(1) == 0) {
         write, "--- child",i,"failed unexpectedly";
      } else {
         same_thing = allof(dotprod(1+i*chunk:(i+1)*chunk,)==xxx);
         write, "     child", i, "ok?", same_thing;
      }
   }
   t_5 = tac();
      
   write, "-- shm dotprod:", t_1+t_2+t_3+t_4+t_5;
   write, "     shm setup:", t_1;
   write, "    sync start:", t_2;
   write, "    my compute:", t_3;
   write, " sync complete:", t_4;
   write, "  shm retreive:", t_5;
   
   // cleanup
   sem_cleanup, my_semid;
   shm_cleanup, my_shmid;
   quit;
} else {
   idstr = swrite(format="data%d",id);
   // wait for parent to tell we're ok to start
   sem_take,my_semid,0, wait=-1; // btw. wait = -1 is the default
   // compute our share
   ours=a(1+id*chunk:(id+1)*chunk,)(,+)*a(+,);
   // write the result back
   shm_write, my_shmid,idstr,&ours;
   // tell the master we're done
   sem_give,my_semid,1;
}

