/*
 *			s e n d v e r s i o n . c
 * $Revision: 131 $
 * $Author: eckertb $
 *
 * RMS Gateway
 *
 * Copyright (c) 2004-2009 Hans-J. Barthen - DL5DI
 * Copyright (c) 2008-2009 Brian R. Eckert - W3SG
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
static char svnid[] = "$Id: sendversion.c 131 2011-11-28 19:31:57Z eckertb $";
#endif /* lint */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>

#include "rms.h"
#include "rmslib.h"

#include "aci.h"


/***
 *  sendVersion(callsign)
 *
 *  send the version info to the winlink system for tracking in
 *  the master database
 *
 *  returns: 0 if version successfully sent, -1 otherwise
 */
int sendVersion(config *cfg)
{
     char versionMessage[MAX_UDP_MSG_LEN];
     char *host;
     char *port;

     /*
      * set parameters - assign defaults if necessary
      */
     if (cfg->gwcall == NULL || strlen(cfg->gwcall) < 3) {
	  syslog(LOG_ERR, "ERROR: sendVersion(): bad callsign (%s)",
		 cfg->gwcall);
	  return(-1);
     }
     
    /*
      * set parameters - assign defaults if necessary
      */
     if (cfg->udpmsghost == NULL || strlen(cfg->udpmsghost) < 1) {
	  host = DEFAULT_UDP_MSG_HOST;
	  syslog(LOG_INFO, "INFO: sendVersion(): assuming destination '%s'",
		 host);
     } else {
	  host = cfg->udpmsghost;
     }

     if (cfg->udpmsgport == NULL || strlen(cfg->udpmsgport) < 1) {
	  port = DEFAULT_UDP_MSG_PORT;
	  syslog(LOG_INFO, "INFO: sendVersion(): assuming port/service '%s'",
		 port);
     } else {
	  port = cfg->udpmsgport;
     }

     snprintf(versionMessage, sizeof(versionMessage),
	      "%s,%.10s,%s,%s",
	      "00", cfg->gwcall, version.program, version.label);

     if (sendDataGram(host, port, versionMessage, strlen(versionMessage)) < 0) {
	  syslog(LOG_ERR, "ERROR: version update message failed");
	  return(-1);
     }

     return(0);
}
