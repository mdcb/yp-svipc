#include "matmult.i"
a=double(random(400,700))
b=double(random(700,900))
tic;
z=a(,+)*b(+,);
t1 = tac();
tic;
k=matmult(a,b);
t2 = tac();
write,"all the same?", allof(k==z);
write,"  timing", t1, t2;
quit;