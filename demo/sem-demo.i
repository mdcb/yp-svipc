#include "svipc.i"

// create ids
my_semid = 0x7dcb0000 | getpid();

// create a pool with one semaphore
sem_init,my_semid,nums=1;

// fork and go
if ( fork() != 0 ) {
   // wait for child, forever
   write, "#1 - wait for child, forever";
   s = sem_take(my_semid, 0);
   write, " ", s;
   
   // wait for child, 500ms
   write,"#2 - wait for child, 500ms (will timeout)";
   s = sem_take(my_semid, 0, wait=0.5);
   write, " ", s;
   
   // wait for child to ack 10 times
   write,"#3 - wait for child 10x";
   s = sem_take(my_semid, 0, count=10);
   write, " ", s;
  
   // cleanup
   sem_cleanup, my_semid;

   // adios
   quit;
} else {
   // child test #1
   pause,1000;
   sem_give,my_semid,0;
   
   // child test #2
   pause,1000;
   sem_give,my_semid,0;
   
   // child test #3
   pause,1000;
   for (i=0;i<10;i++) {
      sem_give,my_semid,0;
   }
   quit;
}
