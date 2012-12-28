/*
 *			c m s l o g i n . c
 * $Revision: 131 $
 * $Author: eckertb $
 *
 * RMS Gateway
 *
 * Copyright (c) 2004-2008 Hans-J. Barthen - DL5DI
 * Copyright (c) 2008 Brian R. Eckert - W3SG
 *
 * Questions or problems regarding this program can be emailed
 * to linux-rmsgw@w3sg.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef lint
static char svnid[] = "$Id: cmslogin.c 131 2011-11-28 19:31:57Z eckertb $";
#endif /* lint */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <syslog.h>

#include "rms.h"
#include "rmslib.h"


static int	timedOut = 0;

/***
 *  onalrm()
 *
 *  expectsend() alarm catcher
 */
static void onalrm(int sig)
{
     timedOut++;
}


/***
 *  expectsend()
 *
 *
 *  returns: 0 on success, -1 or failure, 1 on timeout
 */
static int expectsend(int fd, char *expectStr, char *sendStr, int timeOut)
{
     int nwritten, nread;
     char rcvbuf[2048];
     char *rp = rcvbuf;

     memset(rcvbuf, '\0', sizeof(rcvbuf));

     /*
      * if we expect something, wait for it before sending
      */
     if (expectStr && strlen(expectStr) > 0) {
	  *rp = '\0'; /* initialially an empty buffer */

	  /*
	   * setup alarm handler and set the alarm
	   */
	  timedOut = 0;
	  signal(SIGALRM, onalrm);
	  alarm(timeOut);

	  while (!timedOut && ((char *) strstr(rcvbuf, expectStr) == NULL)) {
	       /*
		* read and display the response from the TNC
		*/
	       if ((nread = read(fd, rp,
				 sizeof(rcvbuf) - (rp - rcvbuf))) < 0) {
		    syslog(LOG_ERR, "expectsend(): read error (errno = %d)",
			   errno);
		    return(-1);
	       }
	       rp += nread;
	       syslog(LOG_DEBUG, "expectsend() read %d bytes, buf = [%s]",
		      nread, rcvbuf);
	  }

	  alarm(0); /* stop the alarm */

	  if (timedOut) {
	       syslog(LOG_WARNING, "WARNING: expectsend(): expect timeout; "
		      "buffer = [%.*s]",
		      (int) (rp - rcvbuf), rcvbuf);
	       return(1);
	  }
	  
	  syslog(LOG_DEBUG, "expectsend() got [%.*s]",
		 (int) (rp - rcvbuf), rcvbuf);
     }

     if ((nwritten = write(fd, sendStr, strlen(sendStr))) < strlen(sendStr)) {
	  if (nwritten < 0) {
	       syslog(LOG_ERR, "expectsend(): write error (errno = %d)",
		      errno);
	       return(-1);
	  } else {
	       syslog(LOG_ERR, "ERROR: expectsend() short write (errno = %d!",
		    errno);
	       return(-1);
	  }
     }

     return(0);
}


int cmslogin(int sd, channel *c, char *usercall, char *passwd)
{
     char sbuf[1024];
     int rc;

     /*
      * make sure we have a channel
      */
     if (c == NULL) {
	  return(-1); /* can't login if no channel info */
     }

     /*
      * do login "chat" via socket to CMS
      */
     snprintf(sbuf, sizeof(sbuf), ".%s\r", usercall);
     if ((rc = expectsend(sd, "Callsign :", sbuf, 15)) != 0) {
	  /*
	   * doesn't matter if it was an error or timeout, we will fail
	   * this login and let the gateway move on
	   */
	  return(-1);
     }

     snprintf(sbuf, sizeof(sbuf), "%s %s %s %s\r",
	     passwd, c->ch_callsign, c->ch_frequency, c->ch_mode);
     if ((rc = expectsend(sd, "Password :", sbuf, 20)) != 0) {
	  /*
	   * just fail it!
	   */
	  return(-1);
     }

     return(0); /* login successful */
}

