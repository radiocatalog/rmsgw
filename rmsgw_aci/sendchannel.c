/*
 *			s e n d c h a n n e l . c
 * $Revision: 142 $
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
static char svnid[] = "$Id: sendchannel.c 142 2012-12-27 20:04:46Z eckertb $";
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
 *  sendChannel(config, channel)
 *
 *  send the channel info to the winlink system for status update
 *  the winlink system
 *
 *  returns: 0 if version successfully sent, -1 otherwise
 */
int sendChannel(config *cfg, channel *chnl)
{
     char channelMessage[MAX_UDP_MSG_LEN];
     char *host;
     char *port;

     /*
      * set parameters - assign defaults if necessary
      */
     if (cfg->udpmsghost == NULL || strlen(cfg->udpmsghost) < 1) {
	  host = DEFAULT_UDP_MSG_HOST;
	  syslog(LOG_INFO, "INFO: sendChannel(): assuming destination '%s'",
		 host);
     } else {
	  host = cfg->udpmsghost;
     }

     if (cfg->udpmsgport == NULL || strlen(cfg->udpmsgport) < 1) {
	  port = DEFAULT_UDP_MSG_PORT;
	  syslog(LOG_INFO, "INFO: sendChannel(): assuming port/service '%s'",
		 port);
     } else {
	  port = cfg->udpmsgport;
     }

     snprintf(channelMessage, sizeof(channelMessage),
	      "%s '%.10s','%.10s','%s',%s,%s,%s,%s,%s,%s,%s,'%s',%s",
	      "02",
	      chnl->ch_callsign,
	      chnl->ch_basecall,
	      chnl->ch_gridsquare,
	      chnl->ch_frequency,
	      chnl->ch_mode,
	      chnl->ch_baud,
	      chnl->ch_power,
	      chnl->ch_height,
	      chnl->ch_gain,
	      chnl->ch_direction,
	      chnl->ch_hours,
	      chnl->ch_groupreference,
	      chnl->ch_servicecode);

     if (sendDataGram(host, port, channelMessage, strlen(channelMessage)) < 0) {
	  syslog(LOG_ERR, "ERROR: channel update message failed");
	  return(-1);
     }

     return(0);
}
