addenv LD_LIBRARY_PATH $PWD
rlwrap yorick

#include "svipc.i"

//a=indgen(10)
//z=shmoobj(&a)
//info,z
//z(10)
//z.toto


shm_var,0xbadcafe,"momo",aaa

shm_read(0xbadcafe,"momo")
shm_free(0xbadcafe,"momo")
shm_cleanup(0xbadcafe)

shm_var,0xbadcafe,"momo",aaa
shm_var,0xbadcafe,"biggie",bbb
shm_unvar,bbb

