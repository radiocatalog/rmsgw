/*
 *			a c i . h
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

#ifndef _aci_h
#define _aci_h	1

#ifndef lint
static char	_aci_h_svnid[] = "$Id: aci.h 131 2011-11-28 19:31:57Z eckertb $";
#endif /* lint */

#define DEFAULT_UDP_MSG_HOST	"winlink.org"
#define DEFAULT_UDP_MSG_PORT	"8778"
#define MAX_UDP_MSG_LEN		512


#ifdef notdef
#define BUFLEN	8192

extern struct ust aci(int s, cmsnode *cmsp, config *cfg, char *ax25port);

/*
 * gateway error returns (ust structure rc codes)
 */
#define ERR_GW_LOGIN		-1
#define ERR_GW_TIMEO		1 /* gatway timeout with CMS */
#define ERR_GW_FATAL		-101 /* complete failure error code bound */
#define ERR_GW_SOCK		-101
#define ERR_GW_AX25		-102
#define ERR_GW_SOCKIO		-103 /* AX.25 to CMS */
#define ERR_GW_AX25IO		-104 /* CMS to AX.25 */
#endif

extern int sendVersion(config *cfg);
extern int sendChannel(config *cfg, channel *chnl);

extern int wl2k_aci(config *cfg, rms_status *sp);

#endif /* _aci_h */
