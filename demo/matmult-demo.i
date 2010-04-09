#include "matmult.i"
a=double(random(1200,1000))
b=double(random(1000,700))
//a=float(random(1200,1000))
//b=float(random(1000,700))
tic;
z=a(,+)*b(+,);
t1 = tac();
write,1," core timing", t1;

maxcore = nprocs();

for (core=2;core<=maxcore;core++) {
   tic;
   k=matmult(a,b,np=core);
   t2 = tac();
   write,core," core timing", t2," check?", allof(k==z);
}

quit;