/*
 *			r m s g w _ a c i . c
 * $Revision: 135 $
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
static char svnid[] = "$Id: rmsgw_aci.c 135 2011-11-30 12:01:49Z eckertb $";
#endif /* lint */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <errno.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "rms.h"
#include "rmslib.h"
#include "mapname.h"

#include "aci.h"

extern int shm_debug;


/***
 *  dohelp()
 */
static void dohelp(char *helpfile)
{
     printf("%s version %s\n",
	    version.package, version.label);
     if (printfile(helpfile) < 0) {
	  fprintf(stderr, "Can't dislay helpfile '%s' (errno = %d)\n",
		  helpfile, errno);
     }
}


/***
 *  main()
 */
int main(int argc, char *argv[])
{
     char *gwcallarg = NULL; /* command line supplied gateway callsign */
     char *logmaskarg = NULL; /* command line over-ride of logmask */
     char *configfilearg = NULL; /* command line over-ride of config file */
     config *cfg;
     time_t now;
     char statfile[PATH_MAX];
     struct stat st;
     int rc; /* generic return code var */
     int opt;
     extern int optind;
     extern char *optarg;
     extern rms_status *rms_stat;

     /*
      * set shared memory activity debugging flag
      */
     shm_debug = set_shm_debug();

     /*
      * intially open the log and set the default log mask
      * to get started and until we get the configuration
      */
     openlog("rmsgw_aci", LOG_CONS|LOG_PID|LOG_NDELAY, LOG_LOCAL0);
     setlogmask(LOG_UPTO(LOG_INFO));

     /*
      * parse the command line options and take appropriate actions
      */
     while ((opt = getopt(argc, argv, "hc:l:g:")) != -1) {
	  switch (opt) {
	  case 'h': /* help */
	       dohelp(ACIHELPFILE);
	       exit(0);
	       break;
	  case 'c': /* config file */
	       configfilearg = strdup(optarg);
	       break;
	  case 'l': /* log mask */
	       logmaskarg = strdup(optarg);
	       break;
	  case 'g': /* gateway call */
	       gwcallarg = strdup(optarg);
	       break;
	  case ':':
	  case '?':
	  default:
	       syslog(LOG_ERR, "Invalid option usage");
	       exit(1);
	  }
     }

     /*
      * get the configuration
      */
     if ((cfg = loadConfig(configfilearg ? configfilearg : configfile,
			   &rc)) == NULL) {
	  switch (rc) {
	  case ERR_CFG_MISSINGFILE:
	       syslog(LOG_ERR, "ERROR: missing config file");
	       break;
	  case ERR_CFG_INVALIDCALL:
	       syslog(LOG_ERR, "ERROR: invalid gateway callsign");
	       break;
	  case ERR_CFG_MISSINGCALL:
	       syslog(LOG_ERR,
		      "ERROR: missing gateway callsign in config");
	       break;
	  case ERR_CFG_MAPERROR:
	       syslog(LOG_ERR,
		      "ERROR: config file: %s", maperrmsg(map_errno));
	       break;
	  default: /* should never happen! */
	       syslog(LOG_CRIT, "UNKNOWN ERROR IN CONFIG FILE");
	       break;
	  }
	  fprintf(stderr, "%s: failed to load configuration. See syslog.\n",
		  argv[0]);
	  exit(1);
     }

     /*
      * replace configured values with any supplied command
      * line over-rides
      */
     if (gwcallarg) { /* gateway call over-ride? */
	  free(cfg->gwcall);
	  cfg->gwcall = gwcallarg;
     }

     if (logmaskarg) { /* logmask over-ride? */
	  free(cfg->logmask);
	  cfg->logmask = logmaskarg;
     }

     /*
      * reopen syslog using the configured facility/logmask
      */
     closelog();
     openlog("rmsgw_aci", LOG_CONS|LOG_PID|LOG_NDELAY,
	     mapfacility(downcase(cfg->logfacility)));
     setlogmask(LOG_UPTO(mappriority(downcase(cfg->logmask))));

     syslog(LOG_INFO,"%s - %s ACI %s %s (%s)",
 	    cfg->gwcall, version.package, version.label,
	    __DATE__, cfg->gridsquare);

     /*
      * check for unconfigured or improperly configured gateway
      * (this check is rather simplistic, but will catch a basic
      *  case of an incomplete configuration)
      */
     if (!strcmp(cfg->gwcall, "N0CALL")) {
	  syslog(LOG_CRIT,
		 "FATAL: Incomplete/incorrect gateway configuration (N0CALL)");
	  exit(1);
     }

     /*
      * attach the status block to the process (attached memory address will
      * be at returned value in rms_stat)
      */
     if (statblock_attach(&rms_stat) < 0) {
	  syslog(LOG_CRIT, "FATAL: cannot attach shared memory segment");

	  fprintf(stderr,
		  "FATAL error: can't attach status block (see syslog)\n");

	  closelog();
	  exit(1);
     }

     /*
      * update status block - set initial condition
      */
     sbset_aci_pid(rms_stat);
     sbset_aci_state(rms_stat, ACI_RUNNING);

     if (wl2k_aci(cfg, rms_stat) < 0) {
	  sbset_aci_state(rms_stat, ACI_IDLE);
	  statblock_detach(rms_stat);
	  exit(1); /* failed aci */
     }/* else... */

     /*
      * check status file for version update
      */
     sprintf(statfile, "%s/.version.%s", CMSSTATDIR, cfg->gwcall);
     if (stat(statfile, &st) < 0) {
	  /* can't stat file, check why */
	  switch (errno) {
	  case ENOENT: /* file doesn't exist */
	       /*
		* proceed
		*/
	       st.st_atime = (time_t) 0;
	       break;
	  default:
	       syslog(LOG_WARNING, "Can't stat %s (errno = %d)",
		      statfile, errno);
	       st.st_atime = (time_t) 0;
	       break; /* pretend we're okay anyway */
	  }
     }


     /*
      * if the version hasn't been updated in an hour or more,
      * send it
      */
     time(&now);
     if ((now - st.st_atime) >= SECSPERHOUR) {
	  sbset_aci_state(rms_stat, ACI_VERSIONUPDATE);
	  sendVersion(cfg);
	  sbset_aci_state(rms_stat, ACI_RUNNING);

	  /*
	   * update the status file
	   */
	  setstatfile(statfile);
     }

     sbset_aci_state(rms_stat, ACI_IDLE);
     statblock_detach(rms_stat);
     exit(0);
}
