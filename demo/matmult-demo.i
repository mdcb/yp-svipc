#include "matmult.i"

// it's not just cpu but also memory
// fudge the max number of proc we fork

fudge = 1; // 1 proc per cpu
fudge = 2; // 2 proc per cpu


//a=double(random(1200,1000))
//b=double(random(1000,700))
a=double(random(1456,1433))
b=double(random(1433,1527))
//a=float(random(1200,1000))
//b=float(random(1000,700))
//a=float(random(1237,1433))
//b=float(random(1433,1113))
tic;
z=a(,+)*b(+,);
t1 = tac();
write,1,"  thread timing", t1;

maxcore = nprocs();

// play ... 
for (core=2;core<=fudge*maxcore;core++) {
   tic;
   k=matmult(a,b,np=core);
   t2 = tac();
   write,core," threads timing", t2," check?", allof(k==z);
}

quit;