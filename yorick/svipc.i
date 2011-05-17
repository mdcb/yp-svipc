/*
 * svipc.i
 *
 * Copyright (c) 2010-2015, Matthieu D.C Bec
 *
 * This program is free software; you can redistribute it and/or  modify it
 * under the terms of the GNU General Public License  as  published  by the
 * Free Software Foundation; either version 2 of the License,  or  (at your
 * option) any later version.
 *
 * This program is distributed in the hope  that  it  will  be  useful, but
 * WITHOUT  ANY   WARRANTY;   without   even   the   implied   warranty  of
 * MERCHANTABILITY or  FITNESS  FOR  A  PARTICULAR  PURPOSE.   See  the GNU
 * General Public License for more details (to receive a  copy  of  the GNU
 * General Public License, write to the Free Software Foundation, Inc., 675
 * Mass Ave, Cambridge, MA 02139, USA).
 *
 * Initial release Matthieu D.C. Bec, April 2010.
 * see release notes in ChangeLog
 * all documentation at http://www.maumae.net/
 *
 *
*/

SVIPC_VERSION = 0.11;

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
   sem_init ................ create a pool of semphores Ids
   sem_cleanup ............. release a pool of semphores Ids
   sem_info ................ print a report on semaphores usage
   sem_take ................ take semaphore Id
   sem_give ................ give (release) semaphore Id
   
   3. message queues
   msq_init ................ create a message queue
   msq_cleanup ............. release a message queue
   msq_info ................ print a report on message queue 
   msq_snd ................. send a message to a message queue
   msq_rcv ................. receive a message from a message queue
   
   4. miscellaneous
   
   ftok .................... generate a System V IPC key
   svipc_debug.............. debug level for the module (int)
   fork..................... fork the current yorick process
   getpid................... get the running process (yorick session) id
   nprocs....................returns the number of processors currently online

 */

plug_in, "svipc";


//---------------------------------------------------------------
// fork
//---------------------------------------------------------------

extern fork;
/* DOCUMENT fork
   fork (clones, with a different pid) the current yorick session.
   The child stdin is replaced by a bogus fd, so is lost.
   The child stdout and stderr are unchanged, so they will appear
   on your terminal.
   This function has mostly applications in parallel problems,
   where one want to fork childs to process operations in
   parallel that would/could have been done with the parent
   process, with the loaded environment.
   Synchronization between the parent and the child processes
   has to be done through shared memory, semaphore and message
   passing, as implemented in this plugin.

   Example:
   func test(void)
   {
     if (fork()!=0) write,"I'm the parent";
     else {
       write,"I'm the child";
       function_to_be_executed_by_the_child;
     }
   }
   > test
   I'm the parent
   I'm the child
   
   AUTHOR: F.Rigaut
   SEE ALSO: spawn
 */


//---------------------------------------------------------------
// getpid
//---------------------------------------------------------------

extern getpid;
/* DOCUMENT getpid()
   Return id of the current yorick session
   AUTHOR: F.Rigaut
*/


//---------------------------------------------------------------
// shm_init
//---------------------------------------------------------------

func shm_init(key, slots=)
{
/* DOCUMENT shm_init(key, slots=)
      (int) key - a System V IPC key
      (int) slots - the number of shared memory segments to create
   Initialize a pool of shared memory identified by 'key' containing
   'slots' initially free segments.
 */
  if (slots==[]) slots=int(-1);
  return Y_shm_init(key, slots);
}
extern Y_shm_init;
/* PROTOTYPE
   void Y_shm_init(int, int)
 */

//---------------------------------------------------------------
// shm_info
//---------------------------------------------------------------

func shm_info(key, details=)
{
/* DOCUMENT shm_info(key, details=)
      (int) key - a System V IPC key
      (int) details - the level of details to print
   Print a report on shared memory pool identified by 'key'.
   'details' controls the level of information printed out.
 */
  if (details==[]) details=int(0);
  return Y_shm_info(key,details);
}
extern Y_shm_info;
/* PROTOTYPE
   void Y_shm_info(int,int)
 */


//---------------------------------------------------------------
// shm_write
//---------------------------------------------------------------

func shm_write(key,id,a,publish=)
{
/* DOCUMENT shm_write(key, id, a, publish=)
      (int) key - a System V IPC key
      (string) id - a slot Id
      (&pointer) a - a yorick variable pointer
      (bool) publish - broadcast to subscribers a new value has been written

   Write the content of the variable referenced by a in the slot
   identified by 'id' from the shared memory pool identified by 'key'.
   
   This operation is semaphore protected and guarantees consistency
   for external readers.
   
   'a' is reference to a yorick variable, usually &variable.
 */
  if (publish==[]) publish=int(0);
  return Y_shm_write(key,id,a,publish);
}
extern Y_shm_write;
/* PROTOTYPE
   void Y_shm_write(int,string,pointer,int)
 */

//---------------------------------------------------------------
// shm_read
//---------------------------------------------------------------

func shm_read(key,id,subscribe=)
{
/* DOCUMENT shm_read(key, id, subscribe=)
      (int) key - a System V IPC key
      (string) id - a slot Id
      (float) subscribe - if set, wait (block) for a publisher broadcast
      
   Read the content of the slot identified by 'id' from the shared
   memory pool identified by 'key'.

   If subscribe > 0, the parameter is understood as a maximum number of
   seconds to wait for a broadcast event, or timeout.
   
   If subscribe < 0, the calling process will block until reception of
   a broadcast.
   
   If subscribe = 0, read the current value from shared memory
   indepently of write broadcast.
   
   This operation is semaphore protected and guarantees consistency
   with external writers.
 */
  if (subscribe==[]) subscribe=float(0);
  return Y_shm_read(key,id,subscribe);
}
extern Y_shm_read;
/* PROTOTYPE
   void Y_shm_read(int,string,float)
 */


//---------------------------------------------------------------
// shm_free
//---------------------------------------------------------------

func shm_free(key,id)
{
/* DOCUMENT shm_free(key,id)
      (int) key - a System V IPC key
      (string) id - a slot Id

   Release the slot identified by 'id' from the shared memory pool
   identified by 'key'.  This operation is semaphore protected and
   guarantees consistency with external readers and writers.
 */
  return Y_shm_free(key,id);
}
extern Y_shm_free;
/* PROTOTYPE
   void Y_shm_free(int,string)
 */

//---------------------------------------------------------------
// shm_cleanup
//---------------------------------------------------------------

func shm_cleanup(key)
{
/* DOCUMENT shm_cleanup(key)
      (int) key - a System V IPC key
      
   Release all the slots from the shared memory pool identified by
   'key'.
   
   This operation is semaphore protected and guarantees consistency
   with external readers and writers.
 */
  return Y_shm_cleanup(key);
}
extern Y_shm_cleanup;
/* PROTOTYPE
   void Y_shm_cleanup(int)
 */


//---------------------------------------------------------------
// shm_var
//---------------------------------------------------------------

extern shm_var;
/* DOCUMENT shm_var, key, id, reference
      (int)   key - master shared memory key
      (string) id  - shared variable lookup name
      reference - an unadorned yorick variable
      
   Binds a new reference variable to the content of the slot
   identified by 'id' from the shared memory pool identified by 'key'.
   
   Access to this new reference is *NOT* semaphore protected,
   consistency with external readers and writers must be handled by
   other means in your application.
*/

//---------------------------------------------------------------
// shm_unvar
//---------------------------------------------------------------

extern shm_unvar;
/* DOCUMENT shm_unvar,reference
      reference - an unadorned yorick variable
      
   Unbinds a new reference variable attached to the slot identified by
   'id' from the shared memory pool identified by 'key'.
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
   void Y_ftok(string,int)
 */

//---------------------------------------------------------------
// nprocs
//---------------------------------------------------------------

extern nprocs;
/* DOCUMENT nprocs
   Returns the number of processors currently online (available).
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
/* DOCUMENT sem_init(key, nums=)
      (int) key - a System V IPC key
      (int) num - the number of semaphores to create
      
   Initialize a pool of semaphores identified by 'key' containing
   'num' initially taken (locked) semaphores.
   NB: nums=0 provides a hacked functionality and reset to 0 all the semaphores
   in the pool.
       nums<0 is equivalent to sem_info.
 */
  if (nums==[]) nums=int(-1);
  return Y_sem_init(key, nums);
}
extern Y_sem_init;
/* PROTOTYPE
   void Y_sem_init(int, int)
 */

//---------------------------------------------------------------
// sem_cleanup
//---------------------------------------------------------------

func sem_cleanup(key)
{
/* DOCUMENT sem_cleanup(key)
      (int) key - a System V IPC key

   Release the pool of semaphores identified by 'key'.
 */
  return Y_sem_cleanup(key);
}
extern Y_sem_cleanup;
/* PROTOTYPE
   void Y_sem_cleanup(int)
 */
//---------------------------------------------------------------
// sem_info
//---------------------------------------------------------------

func sem_info(key, details=)
{
/* DOCUMENT sem_info(key, details=)
      (int) key - a System V IPC key
      (int) details - the level of details to print
      
   Print a report on semaphore pool identified by 'key'.
   'details' controls the level of information printed out.
 */
  if (details==[]) details=int(0);
  return Y_sem_info(key,details);
}
extern Y_sem_info;
/* PROTOTYPE
   void Y_sem_info(int,int)
 */

//---------------------------------------------------------------
// sem_take
//---------------------------------------------------------------

func sem_take(key, id, count=, wait=)
{
/* DOCUMENT sem_take(key, id, count=, wait=)
      (int) key - a System V IPC key
      (int) id - a semaphore Id
      (int) count - the number of operations on the semaphore
      (float) wait - a number of seconds
      
   Decrement semaphore Id by 'count'
   The default, count=1, is equivalent to 'take semaphore Id'.
   
   If wait > 0, the parameter is understood as the maximum number of
   seconds to wait to get hold of the semaphore, or timeout.
   
   If wait < 0, the calling process will block until it can take the
   semaphore.
   
   If wait = 0, returns immediately with a status if the operation
   succeeded or not.
   
 */
  if (count==[]) count=1;
  if (wait==[]) wait=-1.;
  return Y_sem_take(key,id,count,wait);
}
extern Y_sem_take;
/* PROTOTYPE
   void Y_sem_take(int,int,int,float)
 */

//---------------------------------------------------------------
// sem_give
//---------------------------------------------------------------

func sem_give(key,id,count=)
{
/* DOCUMENT sem_give(key, id, count=)
      (int) key - a System V IPC key
      (int) id - a semaphore Id
      (int) count - the number of operations on the semaphore
      
   Increment the semaphore Id by 'count'
   The default, count=1, is equivalent to 'release semaphore Id'.
 */
  if (count==[]) count=1;
  return Y_sem_give(key,id,count);
}
extern Y_sem_give;
/* PROTOTYPE
   void Y_sem_give(int,int,int)
 */


//---------------------------------------------------------------
// msq_init
//---------------------------------------------------------------

func msq_init(key)
{
/* DOCUMENT msq_init(key)
      (int) key - a System V IPC key
      
   Creates a message queue identified by 'key'.
 */
  return Y_msq_init(key);
}
extern Y_msq_init;
/* PROTOTYPE
   void Y_msq_init(int)
 */

//---------------------------------------------------------------
// msq_cleanup
//---------------------------------------------------------------

func msq_cleanup(key)
{
/* DOCUMENT msq_cleanup(key)
      (int) key - a System V IPC key
      
   Release the message queue identified by 'key'.
 */
  return Y_msq_cleanup(key);
}
extern Y_msq_cleanup;
/* PROTOTYPE
   void Y_msq_cleanup(int)
 */
//---------------------------------------------------------------
// msq_info
//---------------------------------------------------------------

func msq_info(key, details=)
{
/* DOCUMENT msq_info(key, details=)
      (int) key - a System V IPC key
      (int) details - the level of details to print
      
   Print a report on the message queue identified by 'key'.
   'details' controls the level of information printed out.
 */
  if (details==[]) details=int(0);
  return Y_msq_info(key,details);
}
extern Y_msq_info;
/* PROTOTYPE
   void Y_msq_info(int,int)
 */

//---------------------------------------------------------------
// msq_snd
//---------------------------------------------------------------

func msq_snd(key,mtype,a,nowait=)
{
/* DOCUMENT msq_snd(key, mtype, a, nowait=)
      (int) key - a System V IPC key
      (long) mtype - a message type id
      (&pointer) a - a yorick variable pointer
      (bool) nowait - a boolean
      
   Sends the content of the variable referenced by a to the message
   queue identified by 'key' with a message type of 'mtype'.
   
   The nowait flag controls if the execution should wait until there
   is space in the message queue to send the message or return with an
   error.
 */
  if (nowait==[]) nowait=0;
  return Y_msq_snd(key,mtype,a,nowait);
}
extern Y_msq_snd;
/* PROTOTYPE
   void Y_msq_snd(int,long,pointer,int)
 */

//---------------------------------------------------------------
// msq_rcv
//---------------------------------------------------------------

func msq_rcv(key,mtype,nowait=)
{
/* DOCUMENT msq_rcv(key, mtype, nowait=)
   To Be Documented.
 */
  if (nowait==[]) nowait=0;
  return Y_msq_rcv(key, mtype, nowait);
}
extern Y_msq_rcv;
/* PROTOTYPE
   void Y_msq_rcv(int,long,int)
 */

