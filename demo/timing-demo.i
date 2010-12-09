#include "svipc.i"

#include "tictac.i"



func bigmatprod(a,b,np=) {
/* DOCUMENT bigmatprod(a,b,np=)
      (array) a,b
      (int) np - the number of processes
   Parallelized array product using np processes
   Make sure the first dimension of 'a' is big
 */
   
   if (np==[]) np = nprocs();

   // check inputs
   dima = dimsof(a);
   dimb = dimsof(b);
   if ( dima(1) != 2 || dimb(1) != 2 ) {
      error,"bigmatprod input array must have dims=2"
   }
   if ( dima(3) != dimb(2) ) {
      error,"bigmatprod array size incompatible"
   }
   
   // split payload - dumb demo
   // we split along the first dimension of 'a' so make sure that one is big
   
   chunk_sz = array(dima(2)/np,np);
   // TODO: max all the chunks rather than max the last one
   // for now we'll assume even the small chunks are big so it can be reasonably ignored
   endchunk = dima(2)-np*chunk_sz(1);
   chunk_sz(0) += endchunk;
   chunk_start = array(0,np);
   for (i=1;i<np;i++) {
      chunk_start(i+1) = sum(chunk_sz(:i));
   }
   
   // create some svipc ids
   my_semid = 0x7dcb0000 | getpid();
   my_shmid = 0x78080000 | getpid();
   
   // create a semaphore to synchronize completion with all the children
   sem_init,my_semid,nums=1;

   // initialize np-1 segments of shared memory, one for each child
   shm_init,my_shmid,slots=np-1;

   // the parent process id is 0
   id=0

   // create our np-1 children
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

   // all processes now compute their share
   sz = chunk_sz(id+1)
   st = chunk_start(id+1)
   ours=a(st+1:st+sz,)(,+)*b(+,);

   if (id==0) {
      // TODO - must be a better way
      // create a dummy array to hold the result
      res = array(a(1)*b(1), [2,dima(2),dimb(3)] );
      
      // write our share
      res(1:sz,)=ours;
      
      // wait for all the children
      sem_take, my_semid, 0, count=np-1;
      
      // reconstruct the result
      for (i=1;i<np;i++) {
         idstr = swrite(format="data%d",i);
         xxx = shm_read(my_shmid,idstr);
         if (dimsof(xxx)(1) == 0) {
            sem_cleanup, my_semid;
            shm_cleanup, my_shmid;
            error, "bigmatprod cannot compute, a child failed unexpectedly";
         } else {
            sz = chunk_sz(i+1);
            st = chunk_start(i+1);
            res(st+1:st+sz,)=xxx;
         }
      }
      // cleanup
      sem_cleanup, my_semid;
      shm_cleanup, my_shmid;
      return res;
   } else {
      idstr = swrite(format="data%d",id);
      // write our result
      shm_write, my_shmid,idstr,&ours;
      // tell the parent process we are done
      sem_give,my_semid,0;
      // quit
      quit;
   }
}

// the demo starts now

svipc_debug=0;

ncore = nprocs();

a=double(random(1245,2430))
b=double(random(2430,1452))

//a=double(random(4200,800))
//b=double(random(800,400))

//a=double(random(5200,800))
//b=double(random(800,200))

//a=double(random(5000,1600))
//b=double(random(1600,200))

// finding the sweet spot = number of parallel processes
// for best timing isn't trivial: cores, L2 cache,
// input size etc. The demo does that iteratively...
// sizeofcache = 3;  // L2 cache in MB
// dima_1 = dimsof(a)(2)
// dima_2 = dimsof(a)(3)
// dimb_1 = dimsof(b)(2)
// dimb_2 = dimsof(b)(3)
// fudge = 1
// sweet_spot = int(fudge*maxcore * (8.*((dima_1 * dimb_2)+(dima_1*dima_2)+(dimb_1*dimb_2))/(1024.*1024.)) / sizeofcache);
// write,"num fork sweet spot", sweet_spot;

num_tries = 30;

otimes = []
oprocs = []
ochksz = []

// take a baseline timing measurment
tic;
z=a(,+)*b(+,);
t1 = tac();
write,1,"  thread timing", t1;

grow,otimes,t1;
grow,oprocs,1;

for (i=0;i<num_tries;i++) {
   nfork = ncore+2*i;
   
   tic;
   k=bigmatprod(a,b,np=nfork);
   t2 = tac();
   write,nfork," threads timing", t2, " ok?", allof(k==z);
   
   grow,otimes,t2;
   grow,oprocs,nfork;
}

window,0,wait=1;
plg,otimes,oprocs,marks=0;
limits;

// quit;