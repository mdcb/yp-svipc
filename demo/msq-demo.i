#include "svipc.i"

   
// create ids
my_msqid = 0xdcb00000 | getpid();
   
// create our queue
msq_init,my_msqid;

// fork and go
if (fork()!=0) {
   // parent sends '123' message
   a=[[1,2],[3,4],[5,6]];
   a;
   msq_snd,my_msqid,123,&a;
   // parent reads '321' message
   b = msq_rcv(my_msqid,321);
   b;
   msq_cleanup, my_msqid;
   // check
   write, "ok?", allof(b==transpose(a));
   // adios
   quit;
} else {
   // child reads '123' message
   aa=msq_rcv(my_msqid,123);
   // transpose the result and send it back
   toto = transpose(aa);
   msq_snd,my_msqid,321,&toto;
   // adios
   quit;
}
