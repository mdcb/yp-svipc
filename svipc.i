plug_in, "svipc";

//-----------------------------------------------------------------------------
// svipc - System V interprocess communication mechanisms
//         .. somewhat 'wrapped'
//         only shm is implemented at the moment
//-----------------------------------------------------------------------------
// 
// ftok - convert a pathname and a project identifier to a System V IPC key
// shm_init(key,[slots])
// shm_info(key)
// shm_write(key,id,&val)
// shm_read(key,id)
// shm_free(key,id)
// shm_cleanup(key)
// shm_var(key,id,reference)


func ftok(path, proj=)
{
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
      reference  - unadorned yorick variable
   SEE ALSO:
*/

//---------------------------------------------------------------
// shm_unvar
//---------------------------------------------------------------

extern shm_unvar;
/* DOCUMENT shm_unvar,reference
      reference  - unadorned yorick variable
   SEE ALSO:
*/


