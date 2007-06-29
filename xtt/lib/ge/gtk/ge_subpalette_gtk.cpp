/* 
 * Proview   $Id: ge_subpalette_gtk.cpp,v 1.3 2007-06-29 09:45:19 claes Exp $
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

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

extern "C" {
#include "co_cdh.h"
#include "co_time.h"
#include "pwr_baseclasses.h"
#include "co_dcli.h"
}
#include "co_wow_gtk.h"

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget_gtk.h"

#include "glow.h"
#include "glow_growctx.h"
#include "glow_growapi.h"
#include "glow_growwidget_gtk.h"

#include "ge_attr.h"
#include "ge_subpalette_gtk.h"

//
// Create the navigator widget
//
SubPaletteGtk::SubPaletteGtk(
	void *xn_parent_ctx,
	GtkWidget *xn_parent_wid,
	char *xn_name,
	GtkWidget **w,
	pwr_tStatus *status) :
  SubPalette( xn_parent_ctx, xn_name, status),
	parent_wid(xn_parent_wid)
{
  form_widget = scrolledbrowwidgetgtk_new(
	SubPalette::init_brow_cb, this, &brow_widget);

  // Create the root item
  *w = form_widget;

  *status = 1;
}

//
//  Delete a nav context
//
SubPaletteGtk::~SubPaletteGtk()
{
  delete brow;
  // XtDestroyWidget( form_widget);
}

void SubPaletteGtk::set_inputfocus( int focus)
{
  // TODO if ( focus) Set border ...
  
  gtk_widget_grab_focus( brow_widget);
}

void SubPaletteGtk::create_popup_menu( char *filename, int x, int y) 
{
  gint wind_x, wind_y;

  strncpy( popup_help_filename, filename, sizeof(popup_help_filename));

  CoWowGtk::PopupPosition( form_widget, x, y, &wind_x, &wind_y);

  popupmenu_x = wind_x;
  popupmenu_y = wind_y;

  GtkMenu *menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  GtkWidget *w = gtk_menu_item_new_with_label( "Help");
  g_signal_connect( w, "activate", 
		    G_CALLBACK(activate_help), this);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), w);
  gtk_widget_show(w);

  //g_signal_connect( menu, "deactivate", 
  //		    G_CALLBACK(popup_unmap_cb), this);
  gtk_menu_popup( menu, NULL, NULL, menu_position_func, 
		  this, 0, gtk_get_current_event_time());

}

void SubPaletteGtk::menu_position_func( GtkMenu *menu, gint *x, gint *y, gboolean *push_in,
					gpointer data)
{
  SubPaletteGtk *subpalette = (SubPaletteGtk *)data;
  *x = subpalette->popupmenu_x;
  *y = subpalette->popupmenu_y;
  *push_in = FALSE;
}

//	Callback from the menu.
void SubPaletteGtk::activate_help( GtkWidget *w, gpointer data)
{
  char helpfile[80] = "$pwr_exe/man_subgraph.dat";
  char topic[200];
  char *s;
  SubPaletteGtk *subpalette = (SubPaletteGtk *)data;

  printf( "Help callback %s\n", subpalette->popup_help_filename);

  if ( (s = strrchr( subpalette->popup_help_filename, '/')))
    strncpy( topic, s+1, sizeof(topic));
  else
    strncpy( topic, subpalette->popup_help_filename, sizeof(topic));
  if ( (s = strrchr( topic, '.')))
    *s = 0;
       
  if ( subpalette->help_cb)
    (subpalette->help_cb)( subpalette->parent_ctx, topic, helpfile);
}
