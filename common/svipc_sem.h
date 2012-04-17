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

#if !defined(SVIPC_SEM_H)
#define SVIPC_SEM_H

#ifdef __cplusplus
extern "C" {
#endif

	int svipc_sem_init(key_t key, int numslots);
	int svipc_sem_cleanup(key_t key);
	int svipc_sem_info(key_t key, int details);
	int svipc_semtake(key_t key, int id, int count, float wait);
	int svipc_semgive(key_t key, int id, int count);

#ifdef __cplusplus
}
#endif
#endif
