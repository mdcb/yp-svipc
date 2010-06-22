#include "svipc.i"

// create ids
my_msqid = 0xdcb00000 | getpid();
   
// create our queue
msq_init,my_msqid;

// fork and go
if (fork()!=0) {
   // parent
   aaa=[[1,2],[3,4],[5,6]];
   
   // sends a message
   msq_snd,my_msqid,123,&aaa;
   
   // waits for reply
   bbb = msq_rcv(my_msqid,321);
   
   // cleanup
   msq_cleanup, my_msqid;
   
   // check result
   write, "pass?", allof(aaa==transpose(bbb));
   // adios
   quit;
} else {
   // fork child does not have stdout
   
   // child waits to read 123 message
   ccc=msq_rcv(my_msqid,123);
   
   // transposes it
   ddd=transpose(ccc);
   
   // and sends it back as 321
   msq_snd,my_msqid,321,&ddd;
   
   // adios
   quit;
}
