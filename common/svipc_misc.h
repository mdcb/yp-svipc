/*
 *    Copyright (C) 2011-2012  Matthieu Bec
 *  
 *    This file is part of yp-svipc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined(SVIPC_MISC_H)
#define SVIPC_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************
 * compat hacks
 *******************************************************************/

#if defined(SVIPC_HACKS)
#include <sys/types.h>		// key_t for OSX
#include <sys/sem.h>

	// quantum of time in microseconds to sleep between timedop tries.
#define SVIPC_USLEEP_QUANTUM 1e3	// 1 ms. fine for scripting
	int semtimedop(int semid, struct sembuf *sops, size_t nsops,
		       struct timespec *timeout);
#endif

/*******************************************************************
 * plugin
 *******************************************************************/

	key_t svipc_ftok(char *path, int proj);
	long svipc_nprocs(void);

/*******************************************************************
 * debug
 *******************************************************************/

	extern int svipc_debug;

#define Debug(level, fmt, ...) { \
    if(level<=svipc_debug) { \
       fprintf (stderr,"(%02d) %15s:%-4d - %15s: ",level,__FILE__,__LINE__,__PRETTY_FUNCTION__); \
       fprintf (stderr, fmt, ## __VA_ARGS__); \
       fflush (stderr); \
    } \
 }

#ifdef __cplusplus
}
#endif
#endif
