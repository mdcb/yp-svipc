#include "svipc.i"

my_semid = 0xdeadbeef;
my_shmid = 0xbadcafe;

//sem_cleanup,my_semid;
//shm_cleanup,my_shmid;


// size of the array we'll multiply
// see README for /proc/sys/kernel/shmmax
my_size = 1536;
my_hlfsize = my_size / 2;

test_case = 1;    // 1=test with ints, else test with floats

// figure who we are
cm_line = get_argv();
if ( cm_line(0) != "dotprod-demo2.i" ) {
   is_master = 0;
} else {
   is_master = 1;
}

// in this demo, the master is busy waiting - you won't see much [...]
func on_stdout_clr(msg) { write,format="\x1b[32m%s\x1b[0m\n",msg; }
func on_stderr_clr(msg) { write,format="\x1b[31m%s\x1b[0m\n",msg; }


if (is_master) {
   // create a random big array
   if (test_case == 1) {
      a=long(random(my_size,my_size)*2^31);
      write, "long test";
   } else {
      a=random(my_size,my_size);   // create a random big array - of floats
      write, "float test";
   }
   
   // take a baseline compute time measurement
   tic;
   dotprod = a(,+)*a(+,)
   t_baseline = tac();
   write, "--- baseline (1 proc) dotprod:", t_baseline
   
   // svipc

   // create 2 semaphores:
   // the 1st one (0) will be used to tell the child it's ok to start
   // the 2nd one (1) will be used to sync the master when the worker completes
   sem_init,my_semid,nums=2;
   
   // create 1 segment of shared memory to share our big array
   shm_init,my_shmid,slots=1;
   
   // spawn the child
   cmd = ["/usr/bin/yorick","-q","-i","dotprod-demo2.i","worker"];
   worker = spawn(cmd, on_stdout_clr, on_stderr_clr);
   
   // put the big demo array in shared memory
   tic;
   shm_write, my_shmid,"momo",&a;
   t_1 = tac();
   
   // tell the worker it's ok to start
   tic;
   sem_give,my_semid,0;
   t_2 = tac();
   
   // compute our half
   tic;
   dotprod1 = a(1:my_hlfsize,)(,+)*a(+,);
   t_3 = tac();
   
   // wait for child's half
   tic;
   sem_take,my_semid,-1;
   t_4 = tac();
   
   // read the child's result (actually we read the whole array, doesn't matter)
   tic;
   xxx = shm_read(my_shmid,"momo");
   t_5 = tac();
   
   if (dimsof(xxx)(1) == 0) {
      write, "--- child failed unexpectedly\n";
   } else {
      write, "---      shm (2 proc) dotprod:", t_1+t_2+t_3+t_4+t_5;
      write, "     shm setup:", t_1;
      write, "    sync start:", t_2;
      write, "    my compute:", t_3;
      write, " sync complete:", t_4;
      write, "  shm retreive:", t_5;
      // check the ops from child (we know ours is good..)
      same_thing2 = allof(dotprod(my_hlfsize+1:,)==xxx(my_hlfsize+1:,))
      write, "            ok?", same_thing2
   }
   sem_cleanup, my_semid;
   shm_cleanup, my_shmid;
   quit;
} else {
   // wait for master to tell we're ok to start
   sem_take,my_semid,0, wait=-1;
   // read the data
   a = shm_read(my_shmid,"momo");
   // compute our half
   a(my_hlfsize+1:,) = a(my_hlfsize+1:,)(,+)*a(+,);
   // write the result back
   shm_write, my_shmid,"momo",&a;
   // tell the master we're done
   sem_give,my_semid,1;
}

