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

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>

#include "svipc_misc.h"

int svipc_debug = 0;

#if defined(__gnu_linux__)
#if !defined(__USE_GNU)
#define __USE_GNU
#endif
#include <sched.h>
//---------------------------------------------------------------
// svipc_setaffinity
//---------------------------------------------------------------
int svipc_setaffinity(int cpu)
{
	int status;
  cpu_set_t  mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu, &mask);
  status = sched_setaffinity(0, sizeof(mask), &mask);
	if (status)
		perror("setaffinity failed");
	return status;
}
#else
int svipc_setaffinity(int cpu)
{
	//perror("setaffinity: platform not supported");
	return -1;
}
#endif

//---------------------------------------------------------------
// svipc_ftok
//---------------------------------------------------------------
key_t svipc_ftok(char *path, int proj)
{
	key_t key = ftok(path, proj);
	if (key == -1)
		perror("ftok failed");
	return key;
}

//---------------------------------------------------------------
// svipc_nprocs
//---------------------------------------------------------------
long svipc_nprocs(void)
{
	// These values may not be standard
	// _SC_NPROCESSORS_CONF - number of processors configured
	// _SC_NPROCESSORS_ONLN - number of processors online
	return sysconf(_SC_NPROCESSORS_ONLN);
}

//---------------------------------------------------------------
// hacks
//---------------------------------------------------------------

#if !defined(__gnu_linux__)
#include <time.h>  /* nanosleep */
#include <errno.h> /* EAGAIN */
int semtimedop(int semid, struct sembuf *sops, size_t nsops,
	       struct timespec *timeout)
{
	int status;
	long time_to_live;

	if (timeout != NULL)
		time_to_live = timeout->tv_sec * 1e9 + timeout->tv_nsec;
	else
		time_to_live = -1;

	if (time_to_live >= 0) {
		// poll hack
		sops->sem_flg |= IPC_NOWAIT;
		// loop while it fails, because it's unavailable, and we have not
		// run out of time. anything else, get out.
		// The order in the next statement matters:
		// - errno is updated by semop
		// - we want to semop at least once, even when timeout=0

		while ((status = semop(semid, sops, nsops))
		       && (errno == EAGAIN)
		       && (time_to_live > 0)) {
			usleep(SVIPC_USLEEP_QUANTUM);
			time_to_live -= SVIPC_USLEEP_QUANTUM * 1e3;	// ns.
		}
	} else {
		// regular semop
		status = semop(semid, sops, nsops);
	}

	return status;
}
#endif
