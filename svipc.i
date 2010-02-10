SVIPC_VERSION = 0.1;

local svipc;
/* DOCUMENT svipc plugin:

   System V inter-process communication
   Available functions (for more info, help,function)
   ftok .................... generate a System V IPC key
   shm_init ................ create a pool of shared memory Ids
   shm_cleanup ............. release a pool of shared memory Ids
   shm_info ................ print a report on pool usage and Ids
   shm_write ............... write to shared memory Id
   shm_read ................ read from shared memory Id
   shm_free ................ release a shared memory Id
   shm_var ................. create a variable bound to shared memory
   shm_unvar ............... destroys a variable bound to shared memory

 */

plug_in, "svipc";


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
// shm_init
//---------------------------------------------------------------

func shm_init(key, slots=)
{
/* DOCUMENT shm_init(key, slots=)
      (long) key - a System V IPC key
      (long) slots - a number of Ids to create
   Initialize a pool of shared memory identified by 'key' containing
   'slots' segments of initially free Ids
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

func shm_write(key,id,a)
{
/* DOCUMENT shm_write(key,id,a)
      (long) key - a System V IPC key
      (string) id - a slot Id
      (&pointer) a - a yorick variable pointer
   Write the content of the variable referenced by a in
   the slot identified by 'id' from the shared memory pool
   identified by 'key'.
   This operation is semaphore protected and guarantees
   consistency for external readers.
   'a' is reference to a yorick variable, usually &variable.
 */
  return Y_shm_write(key,id,a);
}
extern Y_shm_write;
/* PROTOTYPE
   void Y_shm_write(long,string,pointer)
 */

//---------------------------------------------------------------
// shm_read
//---------------------------------------------------------------

func shm_read(key,id)
{
/* DOCUMENT shm_read(key,id)
      (long) key - a System V IPC key
      (string) id - a slot Id
   Read the content of the slot identified by 'id' from the
   shared memory pool identified by 'key'.
   This operation is semaphore protected and guarantees
   consistency with external writers.
 */
  return Y_shm_read(key,id);
}
extern Y_shm_read;
/* PROTOTYPE
   void Y_shm_read(long,string)
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
      (&pointer) a - an unadorned yorick variable reference
   Unbinds a new reference variable attached to the slot
   identified by 'id' from the shared memory pool identified by 'key'.
*/


