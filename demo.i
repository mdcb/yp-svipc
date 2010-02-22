addenv LD_LIBRARY_PATH $PWD
rlwrap yorick

a=array(1.,[3,2048,950,3])
info,a


#include "svipc.i"
shm_init(0xbadcafe,slots=3)
a=random(2,2)
a=double([[1,2],[3,4]])
shm_info(0xbadcafe)
shm_write(0xbadcafe,"momo",&a)
shm_info(0xbadcafe)
shm_info(0xbadcafe,details=1)

b=float(indgen(1000))
shm_write(0xbadcafe,"indi",&b)

a=array(1.,[3,2048,950,3])
info,a
shm_write(0xbadcafe,"biggie",&a)


#include "svipc.i"
shm_init(0xbadcafe)
shm_read(0xbadcafe,"momo")

shm_free(0xbadcafe,"momo")
shm_cleanup(0xbadcafe)


shm_var,0xbadcafe,"momo",aaa
shm_var,0xbadcafe,"biggie",bbb
shm_unvar,bbb


