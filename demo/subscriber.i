#include "svipc.i"

//blocking
func wait_data(void) {
   data = shm_read(0xbadcafe,"momo",subscribe=-1.0);
   data; // to sdout
   set_idler, wait_data;
}

//polling
func poll_data(void) {
   data = shm_read(0xbadcafe,"momo",subscribe=1.0);
   data; // to sdout
   set_idler, poll_data;
}

set_idler, wait_data;
//set_idler, poll_data;