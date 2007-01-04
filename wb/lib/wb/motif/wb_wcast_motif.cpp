/* 
 * Proview   $Id: wb_wcast_motif.cpp,v 1.1 2007-01-04 07:29:02 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 **/

/* wb_wcast_motif.cpp -- Attribute object casting window. */

#include "flow_std.h"

#include <stdio.h>
#include <stdlib.h>

#include "co_cdh.h"
#include "co_time.h"
#include "co_dcli.h"
#include "pwr_baseclasses.h"
#include "wb_ldh.h"
#include "flow_x.h"
#include "co_wow_motif.h"

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/ToggleB.h>
#include <Xm/List.h>
#include <X11/cursorfont.h>
#define XK_MISCELLANY
#include <X11/keysymdef.h>

#include "wb_erep.h"
#include "wb_session.h"
#include "wb_attribute.h"
#include "co_msg.h"
#include "wb_wcast_motif.h"


WCastMotif::WCastMotif( 
	void	*wc_parent_ctx,
	Widget 	wc_parent_wid,
	char 	*wc_name,
	ldh_tSesContext wc_ldhses,
	pwr_sAttrRef wc_aref,
	pwr_tStatus *status
	) :
	WCast(wc_parent_ctx,wc_name,wc_ldhses,wc_aref,status), parent_wid( wc_parent_wid)
{
  wow = new CoWowMotif(wc_parent_wid);

  open_castlist();
  *status = 1;
}

WCastMotif::~WCastMotif()
{
  delete wow;
}


