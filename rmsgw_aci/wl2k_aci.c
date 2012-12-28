/*
 *			w l 2 k _ a c i . c
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
 *
 * Description:
 *	Update the winlink channel status for all defined and active
 *	channels for the gateway
 */
#ifndef lint
static char svnid[] = "$Id: wl2k_aci.c 142 2012-12-27 20:04:46Z eckertb $";
#endif /* lint */

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <syslog.h>
#include <sys/wait.h>

#include "rms.h"
#include "rmslib.h"


/*
 * structure for channel ACI statistics reporting
 */
static struct {
     int read;		/* total channels read */
     int active;	/* total channels active */
     int down;		/* total channels reported down */
     int updated;	/* total channels updated */
     int errors;	/* total channel errors */
} chanstats = {
     0,
     0,
     0,
     0,
     0
};


/***
 *  channel_available(check program)
 *
 *  run program as supplied through the shell. If program
 *  exits with a non-zero status, channel is considered unavailable, zero
 *  indicates channel is available
 *
 *  returns: 1 (true) if channel available, 0 (false) otherwise
 */
static int channel_available(char *check_prog)
{
     int rc, status;

     rc = system(check_prog);

     status = WEXITSTATUS(rc);

     if (status == 0) {
	  syslog(LOG_DEBUG, "%s reports available",
		 check_prog);
	  return(1);
     } else if (status < 0) {
	  syslog(LOG_ERR, "ERROR: failed to run %s",
		 check_prog);
	  return(0);
     } else if (status == 127) {
	  syslog(LOG_ERR, "ERROR: unable to execute %s",
		 check_prog);
	  return(0);
     } else {
	  syslog(LOG_NOTICE, "%s reports unavailable",
		 check_prog);
	  return(0);
     }
}


/***
 *  wl2k_aci(config, statuspointer)
 *
 *  perform auto check-in (update gateway channels
 *  in winlink system)
 *
 *  returns: 0 on success
 */
int wl2k_aci(config *cfg, rms_status *sp)
{
     channel *chnl;

     /*
      * set the channel file if one was specified in the config
      */
     if (cfg->channelfile) {
	  setchanfile(cfg->channelfile);
     }

     /*
      * go through each active channel and update the status
      */
     while ((chnl = getchanent()) != NULL) {
	  chanstats.read++;
	  if (strcasecmp(chnl->ch_active, "yes")) {
	       /*
		* channel is not marked active, so move on
		*/
	       continue;
	  }

	  /*
	   * in no service code is defined, default it
	   */
	  if (chnl->ch_servicecode == NULL && strlen(chnl->ch_servicecode) <= 0) {
	       chnl->ch_servicecode = strdup(DFLT_SERVICECODE); /* need to allocate the string,
								   since the library will free
								   this on subsequent calls */
	  }
	  chanstats.active++;
	  syslog(LOG_INFO, "Channel: %s on %s (%s Hz, mode %s)",
		 chnl->ch_callsign, chnl->ch_name,
		 chnl->ch_frequency, chnl->ch_mode);

#ifdef notdef
	  printf("Found channel '%s' (active=%s)\n",
		 chnl->ch_name, chnl->ch_active ? chnl->ch_active : "(null)");
	  printf("\t      basecall = %s\n", chnl->ch_basecall);
	  printf("\t      callsign = %s\n", chnl->ch_callsign);
	  printf("\t    gridsquare = %s\n", chnl->ch_gridsquare);
	  printf("\t     frequency = %s\n", chnl->ch_frequency);
	  printf("\t          mode = %s\n", chnl->ch_mode);
	  printf("\t      autoonly = %s\n", chnl->ch_autoonly);
	  printf("\t          baud = %s\n", chnl->ch_baud);
	  printf("\t         power = %s\n", chnl->ch_power);
	  printf("\t        height = %s\n", chnl->ch_height);
	  printf("\t          gain = %s\n", chnl->ch_gain);
	  printf("\t     direction = %s\n", chnl->ch_direction);
	  printf("\t         hours = %s\n", chnl->ch_hours);
	  printf("\tgroupreference = %s\n", chnl->ch_groupreference);
	  printf("\t   servicecode = %s\n", chnl->ch_servicecode);
	  printf("\t statuschecker = %s\n", chnl->ch_statuschecker);
#endif

	  /*
	   * channel definition says active, now check actual status
	   * of the channel
	   */
	  if (!channel_available(chnl->ch_statuschecker)) {
	       chanstats.down++;
	       continue; /* do not update this channel, move on */
	  }

	  /*
	   * update the channel status
	   */
	  sbset_aci_state(sp, ACI_CHANNELUPDATE);
	  if (!sendChannel(cfg, chnl)) {
	       chanstats.updated++;
	  } else {
	       chanstats.errors++;
	  }
	  sbset_aci_state(sp, ACI_RUNNING);

     }

     endchanent();

     /*
      * channels updated and errored in status block and log check in stats
      */
     sbset_channel_stats(sp, "winlink.org",
			 chanstats.updated, chanstats.errors);

     syslog(LOG_INFO, "Channel Stats: %d read, %d active, %d down, "
	    "%d updated, %d errors",
	    chanstats.read, chanstats.active, chanstats.down,
	    chanstats.updated, chanstats.errors);

     /*
      * cleanup
      */
     sbset_aci_state(sp, ACI_DISCONNECTED);

     return (0);
}
