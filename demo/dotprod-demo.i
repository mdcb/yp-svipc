#include "svipc.i"

my_shmid = 0xbadcafe;
my_size = 1536; // check your /proc/sys/kernel/shmmax 
my_hlfsize = my_size / 2;

test_case = 1;    // 1=test with ints, else test with floats

// figure who we are

cm_line = get_argv();

if ( cm_line(0) != "dotprod-demo.i" ) {
   is_master = 0;
} else {
   is_master = 1;
}

func on_stdout_clr(msg) {
   write,format="\x1b[32m%s\x1b[0m\n",msg;
}

func on_stderr_clr(msg) {
   write,format="\x1b[31m%s\x1b[0m\n",msg;
}


func start_compute(void) {
   extern a;
   
   //write, "child should be running now ...";
   
   tic;
   shm_write, my_shmid,"momo",&a,publish=1;   // put its value in shared memory
   t_wrt = tac();
   // compute our half
   tic;
   dotprod1 = a(1:my_hlfsize,)(,+)*a(+,);
   t_half1 = tac();
   // write, "master done half", t_half1
   
   // wait for child's half
   tic;
   xxx = shm_read(0xbadcafe,"momo",subscribe=0.2); // timeout after 0.2, if the child was faster than us - or died unexpectedly
   t_reply = tac();
   
   if (dimsof(xxx)(1) == 0) {
      write, "--- child was faster than us (estimate might be biased):", t_wrt+ t_reply + t_half1;
      // assume the former
      xxx = shm_read(0xbadcafe,"momo");
   } else {
      write, "--- shm (2 processes) dotprod:", t_wrt+ t_reply + t_half1;
   }
   
   extern dotprod;
   
   // check the ops from child (we know ours is good..)
   same_thing2 = allof(dotprod(my_hlfsize+1:,)==xxx(my_hlfsize+1:,))
   write, "ok?", same_thing2
   
   shm_cleanup,0xbadcafe;
   
   quit;
}



if (is_master) {
   // if float, some improvement(2x core vs. 1x fp unit?)
   // if integers, speed scales better
   if (test_case == 1) {
      a=int(random(my_size,my_size));      // create a random big array - of ints
      write, "int test";
   } else {
      a=random(my_size,my_size);   // create a random big array - of floats
      write, "float test";
   }
   // baseline compute time
   tic;
   dotprod = a(,+)*a(+,)
   t_baseline = tac();
   write, "--- baseline (1 process) dotprod:", t_baseline
   
   //dotprod1 = a(1:my_hlfsize,)(,+)*a(+,);
   //dotprod2 = a(my_hlfsize+1:,)(,+)*a(+,);
   //same_thing1 = allof(dotprod1==dotprod(1:my_hlfsize,))
   //same_thing2 = allof(dotprod2==dotprod(my_hlfsize+1:,))
   //write, "same_thing:", same_thing1*same_thing2 
   
   // shared mem
   shm_init,my_shmid,slots=1;       // declare one seg. of shared memory to run the demo
   dummy = a * 0;
   shm_write, my_shmid,"momo",&dummy;   // write it the 1st time so child knows about this slot
   
   cmd = ["/usr/bin/yorick","-q","-i","dotprod-demo.i","worker"];
   //cmd = ["/usr/bin/yorick","-i","mult-demo.i","worker"];
   worker = spawn(cmd, on_stdout_clr, on_stderr_clr);
   
   after,1.0,start_compute;
   
} else {
   //write, "worker spawned";
   a = shm_read(0xbadcafe,"momo",subscribe=-1.0); // wait for master..
   //write, "worker got array, computing";
   // compute our half
   tic;
   a(my_hlfsize+1:,) = a(my_hlfsize+1:,)(,+)*a(+,);
   t_half2 = tac();
   tic;
   shm_write, my_shmid,"momo",&a,publish=1;
   t_wrt = tac();
   
   write, "worker half compute time", t_half2;
   //write, "worker wrt time", t_wrt;

}

