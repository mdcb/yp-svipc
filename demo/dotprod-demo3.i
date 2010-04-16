#include "svipc.i"

my_verbose = 1;

my_semid = 0xdeadbeef;
my_shmid = 0xbadcafe;

//sem_cleanup,my_semid;
//shm_cleanup,my_shmid;


// size of the array we'll multiply
// see README for /proc/sys/kernel/shmmax
my_size = 1436;
//my_size = 1024;
//my_size = 512;
//my_size = 256; // still good
//my_size = 128; // faster 1only
my_hlfsize = my_size / 2;

// figure who we are
} else {
   is_master = 1;
}

// in this demo, the master is busy waiting - you won't see much [...]
func on_stdout_clr(msg) { write,format="\x1b[32m%s\x1b[0m\n",msg; }
func on_stderr_clr(msg) { write,format="\x1b[31m%s\x1b[0m\n",msg; }


if (is_master) {
   
   // svipc

   // create 2 semaphores:
   // the 1st one (0) will be used to tell the child it's ok to start
   // the 2nd one (1) will be used to sync the master when the worker completes
   // the 3rd one (2) will be used to tell the child the demo is over
   sem_init,my_semid,nums=3;

   // create 1 segment of shared memory to share our big array
   shm_init,my_shmid,slots=1;

   // spawn the child
   cmd = ["/usr/bin/yorick","-q","-i","dotprod-demo3.i","worker"];
   worker = spawn(cmd, on_stdout_clr, on_stderr_clr);

   usleep, 500; // let the child be born. since master starts with a 1proc baseline, does not matter

   for (test_case=3;test_case>=0;test_case--) {
      // create a random big array
      if (test_case == 0) {
         a=int(random(my_size,my_size)*2^31);
         write, "int test: sizeof(int)", sizeof(int); // create a random big array - of int
      } else if (test_case == 1) {
         a=long(random(my_size,my_size)*2^31); // create a random big array - of long
         write, "long test: sizeof(long)", sizeof(long);
      } else if (test_case == 2) {
         a=float(random(my_size,my_size)); // create a random big array - of float
         write, "float test sizeof(float)", sizeof(float);
      } else if (test_case == 3) {
         a=double(random(my_size,my_size)); // create a random big array - of double
         write, "double test sizeof(double)", sizeof(double);
      }

      // take a baseline compute time measurement
      tic;
      dotprod = a(,+)*a(+,)
      t_baseline = tac();
      write, "--- baseline (1 proc) dotprod:", t_baseline

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
      sem_take,my_semid,1;
      t_4 = tac();

      // read the child's result
      // actually we read the whole array: it does (but not too much) matters
      tic;
      xxx = shm_read(my_shmid,"momo");
      t_5 = tac();

      if (dimsof(xxx)(1) == 0) {
         write, "--- child failed unexpectedly\n";
      } else {
         write, "---      shm (2 proc) dotprod:", t_1+t_2+t_3+t_4+t_5;
         if (my_verbose) {
            write, "     shm setup:", t_1;
            write, "    sync start:", t_2;
            write, "    my compute:", t_3;
            write, " sync complete:", t_4;
            write, "  shm retreive:", t_5;
         }
         // check the ops from child (we know ours is good..)
         same_thing2 = allof(dotprod(my_hlfsize+1:,)==xxx(my_hlfsize+1:,))
         write, "            ok?", same_thing2, " -- type", typeof(xxx);
      }
   
      // release slot before next test
      shm_free,my_shmid,"momo";
   }
   
   // the demo is over
   sem_cleanup, my_semid;
   shm_cleanup, my_shmid;
   quit;
} else {
   while (1) {
      // wait for master to tell we're ok to start
      sem_take,my_semid,0, wait=-1; // btw. wait = -1 is the default
      // read the data
      a = shm_read(my_shmid,"momo");
      // compute our half
      a(my_hlfsize+1:,) = a(my_hlfsize+1:,)(,+)*a(+,);
      // write the result back
      shm_write, my_shmid,"momo",&a;
      // tell the master we're done
      sem_give,my_semid,1;
   }
}

