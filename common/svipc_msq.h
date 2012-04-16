/*
 *    Copyright (C) 2011-2016  Matthieu Bec
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

#if !defined(SVIPC_MSQ_H)
#define SVIPC_MSQ_H

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct svipc_msgbuf {
		long mtype;	/* message type, must be > 0 */
		char mtext[1];	/* pointer to message data   */
	} svipc_msgbuf;

	int svipc_msq_init(key_t key);
	int svipc_msq_cleanup(key_t key);
	int svipc_msq_info(key_t key, int details);
	int svipc_msq_snd(key_t key, struct svipc_msgbuf *sendmsg, size_t msgsz,
			  int nowait);
	int svipc_msq_rcv(key_t key, long mtype, struct svipc_msgbuf **rcvmsg,
			  int nowait);

#ifdef __cplusplus
}
#endif
#endif
