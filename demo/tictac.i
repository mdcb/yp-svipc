/************************************/
/* SYSTEM AND PERFORMANCE functions */
/************************************/

func tic(counterNumber)
/* DOCUMENT tic(counter_number)
 * Marks the beginning of a time lapse
 * ex: tic ; do_something ; tac()
 * will print out the time ellapsed between tic and tac
 * a counter number can optionaly be specified if several
 * counters have to be used in parallel.
 * F.Rigaut 2001/10
 * SEE ALSO: tac
 */
{
  if (counterNumber == []) counterNumber = 1;
  if (counterNumber > 10) error,"tic and tac are limited to 10 time counters !";

  el = array(double,3);
  timer,el;
  _nowtime(counterNumber) = el(3);
}
extern _nowtime;
 _nowtime = array(double,10);



func tac(counterNumber)
/* DOCUMENT tac(counter_number)
 * Marks the end of a time lapse
 * ex: tic ; do_something ; tac()
 * will print out the time ellapsed between tic and tac
 * a counter number can optionaly be specified if several
 * counters have to be used in parallel.
 * F.Rigaut 2001/10
 * SEE ALSO: tic
 */
{
  if (counterNumber == []) counterNumber = 1;

  el = array(double,3);
  timer,el;
  elapsed = el(3)-_nowtime(counterNumber);

  return elapsed;
}



