#include "svipc.i"


func matmult(a,b,np=) {
/* DOCUMENT matmult(a,b,np=)
      (array) a,b
      (int) np - the number of processes
   Parallelized array product using np processes.
 */
   
   // TODO: don't just split among procs but also looking
   // at memory considerations..

   if (np==[]) np = nprocs();

   // check inputs
   dima = dimsof(a);
   dimb = dimsof(b);
   if ( dima(1) != 2 || dimb(1) != 2 ) {
      error,"input array must be two dimensional in matmult"
   }
   if ( dima(3) != dimb(2) ) {
      error,"array size incompatible in matmult"
   }
   
   // split payload
   // TODO: split along the biggest dim
   // for now we'll assume all of them are big and split 'a' in rows
   //bigdima=max(dima(1),dima(2))
   //bigdimb=max(dimb(1),dimb(2))
   //bigdim=max(bigdima,bigdimb)
   
   chunk_sz = array(dima(2)/np,np);
   // TODO: max all the chunks rather than max the last one
   // for now we'll assume all of them are big so it can be reasonably ignored
   endchunk = dima(2)-np*chunk_sz(1);
   chunk_sz(0) += endchunk;
   chunk_start = array(0,np);
   for (i=1;i<np;i++) {
      chunk_start(i+1) = sum(chunk_sz(:i));
   }
   
   // create ids
   my_semid = 0xdcb00000 | getpid();
   my_shmid = 0x80800000 | getpid();
   
   // create a semaphore to synchronize completion with all the children
   sem_init,my_semid,nums=1;

   // initialize np-1 segments of shared memory for each child
   shm_init,my_shmid,slots=np-1;

   // the parent process id starts at 0
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

   // all processes compute their share of the product
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
            error, "a child failed unexpectedly in matmult";
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