SVIPC_VERSION = 0.4;

local svipc;
/* DOCUMENT svipc plugin:

   System V inter-process communication
   Available functions (for more info, help,function)
   
   1. shared memory
   
   shm_init ................ create a pool of shared memory Ids
   shm_cleanup ............. release a pool of shared memory Ids
   shm_info ................ print a report on shared memory pool usage
   shm_write ............... write to shared memory Id
   shm_read ................ read from shared memory Id
   shm_free ................ release a shared memory Id
   shm_var ................. create a variable bound to shared memory
   shm_unvar ............... destroys a variable bound to shared memory

   2. semaphores
   sem_init ................ create a pool of semphores
   sem_cleanup ............. release a pool of semphores
   sem_info ................ print a report on semaphores usage
   sem_take ................ take semaphore Id
   sem_give ................ give (release) semaphore Id
   
   3. miscellaneous
   
   ftok .................... generate a System V IPC key
   svipc_debug.............. debug level for the module (int)

 */

plug_in, "svipc";


//---------------------------------------------------------------
// shm_init
//---------------------------------------------------------------

func shm_init(key, slots=)
{
/* DOCUMENT shm_init(key, slots=)
      (long) key - a System V IPC key
      (long) slots - the number of shared memory segments to create
   Initialize a pool of shared memory identified by 'key' containing
   'slots' initially free segments.
 */
  if (slots==[]) slots=long(-1);
  return Y_shm_init(key, slots);
}
extern Y_shm_init;
/* PROTOTYPE
   void Y_shm_init(long, long)
 */

//---------------------------------------------------------------
// shm_info
//---------------------------------------------------------------

func shm_info(key, details=)
{
/* DOCUMENT shm_info(key, details=)
      (long) key - a System V IPC key
      (long) details - the level of details to print
   Print a report on shared memory pool identified by 'key'.
   'details' controls the level of information printed out.
 */
  if (details==[]) details=long(0);
  return Y_shm_info(key,details);
}
extern Y_shm_info;
/* PROTOTYPE
   void Y_shm_info(long,long)
 */


//---------------------------------------------------------------
// shm_write
//---------------------------------------------------------------

func shm_write(key,id,a,publish=)
{
/* DOCUMENT shm_write(key,id,a,publish=)
      (long) key - a System V IPC key
      (string) id - a slot Id
      (&pointer) a - a yorick variable pointer
      (bool) publish - broadcast to subscribers a new value has been written
   Write the content of the variable referenced by a in
   the slot identified by 'id' from the shared memory pool
   identified by 'key'.
   This operation is semaphore protected and guarantees
   consistency for external readers.
   'a' is reference to a yorick variable, usually &variable.
 */
  if (publish==[]) publish=int(0);
  return Y_shm_write(key,id,a,publish);
}
extern Y_shm_write;
/* PROTOTYPE
   void Y_shm_write(long,string,pointer,int)
 */

//---------------------------------------------------------------
// shm_read
//---------------------------------------------------------------

func shm_read(key,id,subscribe=)
{
/* DOCUMENT shm_read(key,id,subscribe=)
      (long) key - a System V IPC key
      (string) id - a slot Id
      (float) subscribe - if set, wait (block) for a publisher broadcast
   Read the content of the slot identified by 'id' from the
   shared memory pool identified by 'key'.
   If subscribe >0, the parameter is understood as a maximum number of seconds
   to wait for a broadcast event, or timeout.
   If subscribe <0, the calling process will block until reception of a
   broadcast.
   If subscribe =0, read the current value from shared memory indepently of
   write broadcast.
   This operation is semaphore protected and guarantees
   consistency with external writers.
 */
  if (subscribe==[]) subscribe=float(0);
  return Y_shm_read(key,id,subscribe);
}
extern Y_shm_read;
/* PROTOTYPE
   void Y_shm_read(long,string,float)
 */


//---------------------------------------------------------------
// shm_free
//---------------------------------------------------------------

func shm_free(key,id)
{
/* DOCUMENT shm_free(key,id)
      (long) key - a System V IPC key
      (string) id - a slot Id
   Release the slot identified by 'id' from the
   shared memory pool identified by 'key'.
   This operation is semaphore protected and guarantees
   consistency with external readers and writers.
 */
  return Y_shm_free(key,id);
}
extern Y_shm_free;
/* PROTOTYPE
   void Y_shm_free(long,string)
 */

//---------------------------------------------------------------
// shm_cleanup
//---------------------------------------------------------------

func shm_cleanup(key)
{
/* DOCUMENT shm_cleanup(key)
      (long) key - a System V IPC key
   Release all the slots from the shared memory pool
   identified by 'key'.
   This operation is semaphore protected and guarantees
   consistency with external readers and writers.
 */
  return Y_shm_cleanup(key);
}
extern Y_shm_cleanup;
/* PROTOTYPE
   void Y_shm_cleanup(long)
 */


//---------------------------------------------------------------
// shm_var
//---------------------------------------------------------------

extern shm_var;
/* DOCUMENT shm_var,key,id,reference
      (long)   key - master shared memory key
      (string) id  - shared variable lookup name
      reference - an unadorned yorick variable
   Binds a new reference variable to the content of the slot
   identified by 'id' from the shared memory pool identified by 'key'.
   Access to this new reference is *NOT* semaphore
   protected, consistency with external readers and writers
   must be handled by other means in your application. 
*/

//---------------------------------------------------------------
// shm_unvar
//---------------------------------------------------------------

extern shm_unvar;
/* DOCUMENT shm_unvar,reference
      reference - an unadorned yorick variable
   Unbinds a new reference variable attached to the slot
   identified by 'id' from the shared memory pool identified by 'key'.
*/



//---------------------------------------------------------------
// ftok
//---------------------------------------------------------------

func ftok(path, proj=)
{
/* DOCUMENT ftok(path, proj=)
      (string) path - a unix file path
      (int) proj    - a project number (default=0)
   Convert a pathname and a project identifier to a System V IPC key
 */
  if (proj==[]) proj=int(0);
  return Y_ftok(path,proj);
}
extern Y_ftok;
/* PROTOTYPE
   long Y_ftok(string,int)
 */

//---------------------------------------------------------------
// svipc_debug
//---------------------------------------------------------------

extern svipc_debug;
/* EXTERNAL svipc_debug */
reshape, svipc_debug, int;




//---------------------------------------------------------------
// sem_init
//---------------------------------------------------------------

func sem_init(key, nums=)
{
/* DOCUMENT sem_init(key, num=)
      (long) key - a System V IPC key
      (long) num - the number of semaphores to create
   Initialize a pool of semaphores identified by 'key' containing
   'num' initially taken (locked) semaphores.
 */
  if (nums==[]) nums=long(-1);
  return Y_sem_init(key, nums);
}
extern Y_sem_init;
/* PROTOTYPE
   void Y_sem_init(long, long)
 */

//---------------------------------------------------------------
// sem_cleanup
//---------------------------------------------------------------

func sem_cleanup(key)
{
/* DOCUMENT sem_cleanup(key)
      (long) key - a System V IPC key
   Release the pool of semaphores identified by 'key'.
 */
  return Y_sem_cleanup(key);
}
extern Y_sem_cleanup;
/* PROTOTYPE
   void Y_sem_cleanup(long)
 */
//---------------------------------------------------------------
// sem_info
//---------------------------------------------------------------

func sem_info(key, details=)
{
/* DOCUMENT sem_info(key, details=)
      (long) key - a System V IPC key
      (long) details - the level of details to print
   Print a report on semaphore pool identified by 'key'.
   'details' controls the level of information printed out.
 */
  if (details==[]) details=long(0);
  return Y_sem_info(key,details);
}
extern Y_sem_info;
/* PROTOTYPE
   void Y_sem_info(long,long)
 */

//---------------------------------------------------------------
// sem_take
//---------------------------------------------------------------

func sem_take(key,id,wait=)
{
/* DOCUMENT sem_take(key,id,wait=)
      (long) key - a System V IPC key
      (long) id - a semaphore Id
      (float) wait - a number of seconds
   If wait >0, the parameter is understood as the maximum number of seconds
   to wait to get hold of the semaphore, or timeout.
   If subscribe <0, the calling process will block until it can take the
   semaphore.
   If subscribe =0, returns immediately with a status if the operation
   succeeded or not.
   
 */
  if (wait==[]) wait=-1.;
  return Y_sem_take(key,id,wait);
}
extern Y_sem_take;
/* PROTOTYPE
   void Y_sem_take(long,long,float)
 */

//---------------------------------------------------------------
// sem_give
//---------------------------------------------------------------

func sem_give(key,id)
{
/* DOCUMENT sem_give(key,id)
      (long) key - a System V IPC key
      (long) id - a semaphore Id
   Release the semaphore Id.
 */
  return Y_sem_give(key,id);
}
extern Y_sem_give;
/* PROTOTYPE
   void Y_sem_give(long,long)
 */

