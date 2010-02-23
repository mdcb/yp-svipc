#include "svipc.i"
shm_init(0x0d0d0d0d,slots=3)
a=random(2,2)
shm_write(0x0d0d0d0d,"a",&a)
shm_info(0x0d0d0d0d,details=1)
shm_read(0x0d0d0d0d,"a")

b=random(2,2)
shm_write(0x0d0d0d0d,"a",&b)

b=random(2,3)
shm_write(0x0d0d0d0d,"a",&b)

b=int(random(2,2))
shm_write(0x0d0d0d0d,"a",&b)

b="ABBA"
shm_write(0x0d0d0d0d,"a",&b)

shm_cleanup(0x0d0d0d0d)


-----
#include "svipc.i"
shm_init(0x0d0d0d0d)
shm_info(0x0d0d0d0d,details=1)
shm_read(0x0d0d0d0d,"a")
