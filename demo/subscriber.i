#include "svipc.i"

func wait_data(void) {
   data = shm_read(0xbadcafe,"momo",subscribe=1);
   data; // to sdout
   set_idler, wait_data;
}

set_idler, wait_data;