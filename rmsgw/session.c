/*
 *			s e s s i o n . c
 * $Revision: 131 $
 * $Author: eckertb $
 *
 * RMS Gateway
 *
 * Copyright (c) 2004-2011 Hans-J. Barthen - DL5DI
 * Copyright (c) 2008-2011 Brian R. Eckert - W3SG
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
static char svnid[] = "$Id: session.c 131 2011-11-28 19:31:57Z eckertb $";
#endif /* lint */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

#include "rmsgw.h"


/***
 *  cmsSession(the_cms)
 *
 *  start a session with the indicated cms
 *
 *  the_cms = info for cms host to be tried
 */
int cmsSession(cmsnode *the_cms, config *cfg, char *ax25port, char *usercall)
{
     int cmsSock;
     struct ust userstat;
     time_t tstart;
     time_t tstop;
     char buffer[MAXBUF];
     int rc = 0;
     channel *chnl;
     extern rms_status *rms_stat;

     /*
      * check if we have a valid port before going any further, as a connection
      * to the CMS is wasted if there is no port when we get here
      */
     if ((ax25port == NULL) || (strlen(ax25port) <= 0)) {
	  syslog(LOG_ERR,
		 "ERROR: no port supplied for channel in session startup");
	  return(ERR_SESS_FATAL); /* we should try no more, since
				     there is no hope of identifying
				     the channel */
     }
	  
     /*
      * set the channel file if one was specified in the config
      */
     if (cfg->channelfile) {
	  setchanfile(cfg->channelfile);
     }

     /*
      * locate the channel information for the port/call
      */
     if ((ax25port != NULL) && (strlen(ax25port) > 0)) {
	  if ((chnl = getchannam(ax25port, cfg->gwcall)) == NULL) {
	       syslog(LOG_ERR,
		      "ERROR: no channel data for port %s, call %s",
		      ax25port, cfg->gwcall);
	       return(ERR_SESS_FATAL);
	  } else {
	       syslog(LOG_INFO, "Channel: %s on %s (%s Hz, mode %s)",
		      chnl->ch_callsign, chnl->ch_name,
		      chnl->ch_frequency, chnl->ch_mode);
	  }
     } else {
	  syslog(LOG_ERR, "ERROR: no port supplied for channel");
	  return(ERR_SESS_FATAL);
     }

     /*
      * attempt to obtain a connected socket to the cms host
      */
     sbset_gw_state(rms_stat, GW_CONNECTING, cfg->gwcall);
     if ((cmsSock = cmsConnect(the_cms)) < 0) {
	  return(ERR_SESS_CONNECT); /* that didn't work */
     }

     sbset_gw_state(rms_stat, GW_CONNECTED, cfg->gwcall);
     syslog(LOG_DEBUG, "CMS Connected");

     time(&tstart);
     log_logon(usercall, ax25port, the_cms->cms_host);
     userstat = gateway(cmsSock, cfg->gwcall, usercall,
			the_cms->cms_passwd, chnl);
     time(&tstop);

     /*
      * check result
      */
     if(userstat.errcode >= 0) {
	  sb_add_gw_bytes_out(rms_stat, cfg->gwcall, userstat.bytes_sent);
	  sb_add_gw_bytes_in(rms_stat, cfg->gwcall, userstat.bytes_recv);
	  snprintf(buffer, sizeof(buffer),
		   "; Sent: %li Bytes / Received: "
		   "%li Bytes, %.1fs, %.1f Bytes/s (%i)\n",
		   userstat.bytes_sent, userstat.bytes_recv,
		   difftime(tstop,tstart),
		   (userstat.bytes_sent+userstat.bytes_recv) / difftime(tstop,
									tstart),
		   userstat.errcode);
	  sendrf(STDOUT_FILENO, buffer, strlen(buffer), SRF_EOLXLATE);

	  snprintf(buffer, sizeof(buffer), "; %s de %s SK\n", usercall, cfg->gwcall);
	  sendrf(STDOUT_FILENO, buffer, strlen(buffer), SRF_EOLXLATE);
	  log_logout(usercall, userstat, tstart, tstop);
     } else if (userstat.errcode <= ERR_GW_FATAL) { /* can't continue */
	  syslog(LOG_ERR, "FATAL ERROR: gateway cannot continue after %s",
		 the_cms->cms_host);
	  rc = ERR_SESS_FATAL; /* complete failure of gateway */
     } else {
	  syslog(LOG_ERR, "ERROR: gateway with %s FAILED!",
		 the_cms->cms_host);
	  rc = ERR_SESS_GWSKIP; /* failed, try next host, if possible */
     }

     sbset_gw_state(rms_stat, GW_DISCONNECTING, cfg->gwcall);
     close(cmsSock);
     sbset_gw_state(rms_stat, GW_DISCONNECTED, cfg->gwcall);
     syslog(LOG_DEBUG, "CMS Disconnected");

     return(rc);
}
