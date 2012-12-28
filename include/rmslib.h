/*
 *			r m s l i b . h
 * $Revision: 142 $
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
#ifndef _rmslib_h
#define _rmslib_h	1

#ifndef lint
static char	_rmslib_h_svnid[] = "$Id: rmslib.h 142 2012-12-27 20:04:46Z eckertb $";
#endif /* lint */

#include "set.h"
#include "rms.h"
#include "rmsshm.h"

/*
 * macros
 */
#ifndef MAX
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#endif

#ifndef STREQUAL
#define STREQUAL(a, b)  (!strcmp((a), (b)))
#endif

#ifndef STRNEQUAL
#define STRNEQUAL(a, b, n)      (!strncmp((a), (b), (n)))
#endif

#ifndef STRNCMP
#define STRNCMP(a, b)   (strncmp((a), (b), strlen(a)))
#endif

/*
 * package version structure (see ../lib/version.c for actual
 * use)
 */
typedef struct _version {
     const char *package; /* the package name */
     const char *program; /* the program name for version reporting to winlink */
     const char *label; /* the version label string */
     /* the remaining items reflect subversion
        keyword values */
     const char *revision; /* the repository revision */
     const char *date; /* revision date */
     const char *author; /* revision author */
     const char *id; /* revision id string */
} version_blk;
extern version_blk version;

/*
 * structure to hold CMS host entries as read from
 * host file
 *
 * host file structure is a set of lines formatted like:
 *	cms-hostname:connection-port:host-password
 */
typedef struct _cms {
     char *cms_host;	/* CMS host name */
     int  cms_port;	/* CMS connect port */
     char *cms_passwd;	/* CMS host password */
} cms;
extern void setcmsfile(char *cp);
extern int setcmsent(void);
extern void endcmsent(void);
extern cms *getcmsent(void);
extern cms *getcmsnam(register char *name);
extern void putcmsent(register cms *c, register FILE *fp);

/*
 * cms host linked list stucture and function prototypes
 */
typedef struct cmsnode {
     char *cms_host;
     int cms_port;
     char *cms_passwd;
     time_t cms_stat_time;
     struct cmsnode *next;

} cmsnode;

extern cmsnode *getcmslist(void);
extern cmsnode *addcmsnode(cmsnode *clist, cms *c, time_t stime);

/*
 * structure to hold channel entries as read from
 * channel XML file
 *
 * RMS channels structure and library function prototypes
 */
typedef struct _channel {
     char *ch_name;	/* port/channel name (e.g., port from ax25ports) */
     char *ch_type;	/* type of channel (e.g., ax25) */
     char *ch_active;	/* channel active indicator (yes/no) */
     char *ch_basecall;	/* base callsign of the gateway */
     char *ch_callsign;	/* callsign/ssid for the gw channel */
     char *ch_gridsquare;/*gridsquare of gateway */
     char *ch_frequency;/* the channel's frequency (in Hz) */
     char *ch_mode;	/* the "pactor" mode of the channel */
     char *ch_autoonly;	/* auto only indicator (0 or 1) */
     char *ch_baud;	/* baud rate of channel */
     char *ch_power;	/* power (watts) of channel */
     char *ch_height;	/* antenna height */
     char *ch_gain;	/* antenna gain in db (0 is unity) */
     char *ch_direction;/* antenna direction (0 is omni) */
     char *ch_hours;	/* hour of operation for this channel */
     char *ch_groupreference;/* channel group reference */
     char *ch_servicecode; /* channel service code */
     char *ch_statuschecker;/* pathname of local channel status check progam */
} channel;
extern void setchanfile(char *p);
extern int setchanent(void);
extern void endchanent(void);
extern channel *getchanent(void);
extern channel *getchannam(register char *chname, register char *callsign);

/*
 * configuration structure used by loadConfig()
 */
typedef struct _config {
     char *gwcall;	/* gateway callsign (configured default) */
     char *channelfile;	/* channel file for login/status reporting */
     char *gridsquare;	/* gateway's grid square */
     char *bannerfile;	/* banner file to be sent once connected */
     char *authfile;	/* rms authorization file for -- no longer used */
     char *rmskeycode;	/* gateway's assigned keycode for winlink system -- no longer needed */
     char *rmspassword;	/* gateway's password (for base callsign) -- no longer needed*/
     char *logfacility;	/* syslog facility name */
     char *logmask;	/* syslog log maks */
     char *udpmsghost;	/* host to which UDP messages are sent */
     char *udpmsgport;	/* port/service for UDP message sending */
} config;

/*
 * possible error codes for loadConfig()
 */
#define ERR_CFG_MAPERROR	1
#define ERR_CFG_MISSINGFILE	2
#define ERR_CFG_INVALIDCALL	3
#define ERR_CFG_MISSINGCALL     4

extern config *loadConfig(const char *cfgfile, int *rc);

/*
 * miscellaneous function prototypes
 */
extern char *strtrim(char *dest, char *src, char *set, int trim);
extern char *upcase(char *s);
extern char *downcase(char *s);
extern char *strcvt(char *s, char f, char t);
extern char *fgetline(char *s, int size, FILE *fp);
extern char *fgetlinecr(char *s, int size, FILE *fp);
extern int printfile(char *file);

extern int cmsConnect(cmsnode *c);
extern int cmslogin(int sd, channel *c, char *usercall, char *passwd);
extern int setcmsstat(cmsnode *p);

extern int mappriority(char *pri);
extern int mapfacility(char *facility);

extern void err(char *message);
extern void msg(char *message);

/*
 * options and prototype for sendrf()
 */
#define SRF_EOLXLATE	0x00000001 /* translate UNIX newline */
#define SRF_ADDEOL	0x00000002 /* add a final end of line on send */

extern int sendrf(int sd, register char *buf, register int len, int opts);

extern int sendDataGram(char *host, char *port, unsigned char* msg, int msg_len);

/*
 * status block function prototypes
 */
extern int set_shm_debug(void);
extern int get_cms_slot(rms_status *p, char *cms_host);
extern int statblock_attach(rms_status **addr);
extern int statblock_detach(rms_status *addr);
extern void sbset_aci_state(rms_status *p, ACISTATE s);
extern void sbset_aci_pid(rms_status *p);
extern void sb_incr_aci_error(rms_status *p, char *cms_host);
extern void sb_incr_aci_checkin(rms_status *p, char *cms_host);
extern void sbset_channel_stats(rms_status *p, char *cms_host, int updates, int errors);
extern void sbset_gw_state(rms_status *p, ACISTATE s, char *gwcall);
extern void sbset_gw_pid(rms_status *p, char *gwcall);
extern void sbset_gw_usercall(rms_status *p, char *gwcall, char *usercall);
extern void sb_incr_gw_connects(rms_status *p, char *gwcall);
extern void sb_add_gw_bytes_in(rms_status *p, char *gwcall, long count);
extern void sb_add_gw_bytes_out(rms_status *p, char *gwcall, long count);

#endif /* _rmslib_h */
