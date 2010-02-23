#include "svipc.i"

shm_init,0xbadcafe,slots=3;

a=random(2,2);
shm_write(0xbadcafe,"momo",&a);

func on_stdout_clr(msg) {
   write,format="\x1b[32m%s\x1b[0m",msg;
}

func on_stderr_clr(msg) {
   write,format="\x1b[31m%s\x1b[0m",msg;
}

cmd = ["/usr/bin/yorick","-i","subscriber.i"];

worker = spawn(cmd, on_stdout_clr, on_stderr_clr);

func daloop(void) {
   a=random(2,2);
   shm_write,0xbadcafe,"momo",&a,publish=1;
   a; // stdout
   after,0.01,daloop;
}

daloop;