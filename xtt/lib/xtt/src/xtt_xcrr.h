/* 
 * Proview   $Id: xtt_xcrr.h,v 1.3 2005-09-01 14:57:48 claes Exp $
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
 */

#ifndef wb_xcrr_h
#define wb_xcrr_h

/* wb_xcrr.h -- Object crossreferences */

#if defined __cplusplus
extern "C" {
#endif

#ifndef pwr_h
# include "pwr.h"
#endif


class XCrr {
  public:
    XCrr( 
	Widget 		xa_parent_wid, 
	void 		*xa_parent_ctx, 
	pwr_sAttrRef 	*xa_objar,
	int 		xa_advanced_user,
	int             *xa_sts);
    ~XCrr();
    Widget	parent_wid;
    void 	*parent_ctx;
    pwr_sAttrRef objar;
    char 	name[80];
    Widget	brow_widget;
    Widget	form_widget;
    Widget	toplevel;
    void	*xcrrnav;
    void	*root_item;
    Widget	xcrrnav_form;
    int		input_open;
    int         input_multiline;
    void 	*object;
    void	(*close_cb) ( void *, void *);
    void	(*redraw_cb) (void *);
    void 	(*popup_menu_cb)( void *, pwr_sAttrRef, unsigned long,
				  unsigned long, char *, Widget * );
    void 	(*start_trace_cb)( void *, pwr_tObjid, char *);
    void	*client_data;
    brow_tObject input_node;
    char	input_name[80];
    int		set_focus_disabled;
    XtIntervalId focus_timerid;

    void pop();
};


#if defined __cplusplus
}
#endif
#endif







