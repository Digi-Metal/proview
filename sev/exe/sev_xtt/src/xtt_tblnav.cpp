/* 
 * Proview   $Id: xtt_tblnav.cpp,v 1.3 2008-09-18 14:37:43 claes Exp $
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

/* ge_attrnav.cpp -- Display object info */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector.h>

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "co_cdh.h"
#include "co_time.h"
#include "co_dcli.h"
#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_msg.h"
#include "xtt_tblnav.h"

#include "xnav_bitmap_leaf12.h"
#include "xnav_bitmap_map12.h"
#include "xnav_bitmap_openmap12.h"
#include "xnav_bitmap_attr12.h"
#include "xnav_bitmap_attrarra12.h"

#define TBLNAV__INPUT_SYNTAX 2
#define TBLNAV__OBJNOTFOUND 4
#define TBLNAV__STRINGTOLONG 6
#define TBLNAV__ITEM_NOCREA 8
#define TBLNAV__SUCCESS 1

void TblNav::message( char sev, char *text)
{
  (message_cb)( parent_ctx, sev, text);
}

void TblNav::print( char *filename)
{
  brow_Print( brow->ctx, filename);
}

//
//  Free pixmaps
//
void TblNavBrow::free_pixmaps()
{
  brow_FreeAnnotPixmap( ctx, pixmap_leaf);
  brow_FreeAnnotPixmap( ctx, pixmap_map);
  brow_FreeAnnotPixmap( ctx, pixmap_openmap);
  brow_FreeAnnotPixmap( ctx, pixmap_attr);
}

//
//  Create pixmaps for leaf, closed map and open map
//
void TblNavBrow::allocate_pixmaps()
{
  flow_sPixmapData pixmap_data;
  int i;
  
  for ( i = 0; i < 9; i++) {
    pixmap_data[i].width =xnav_bitmap_leaf12_width;
    pixmap_data[i].height =xnav_bitmap_leaf12_height;
    pixmap_data[i].bits = (char *)xnav_bitmap_leaf12_bits;
  }
  
  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_leaf);
  
  for ( i = 0; i < 9; i++) {
    pixmap_data[i].width =xnav_bitmap_map12_width;
    pixmap_data[i].height =xnav_bitmap_map12_height;
    pixmap_data[i].bits = (char *)xnav_bitmap_map12_bits;
  }

  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_map);
  
  for ( i = 0; i < 9; i++) {
    pixmap_data[i].width =xnav_bitmap_openmap12_width;
    pixmap_data[i].height =xnav_bitmap_openmap12_height;
    pixmap_data[i].bits = (char *)xnav_bitmap_openmap12_bits;
  }

  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_openmap);

  for ( i = 0; i < 9; i++) {
    pixmap_data[i].width =xnav_bitmap_attr12_width;
    pixmap_data[i].height =xnav_bitmap_attr12_height;
    pixmap_data[i].bits = (char *)xnav_bitmap_attr12_bits;
  }

  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_attr);

  for ( i = 0; i < 9; i++) {
    pixmap_data[i].width =xnav_bitmap_attrarra12_width;
    pixmap_data[i].height =xnav_bitmap_attrarra12_height;
    pixmap_data[i].bits = (char *)xnav_bitmap_attrarra12_bits;
  }
	  
  brow_AllocAnnotPixmap( ctx, &pixmap_data, &pixmap_attrarray);
  
}


//
// Create the navigator widget
//
TblNav::TblNav(
	void *xn_parent_ctx,
	sevcli_sHistItem  *xn_itemlist,
	int xn_item_cnt,
	pwr_tStatus *status) :
	parent_ctx(xn_parent_ctx),
	itemlist(xn_itemlist),item_cnt(xn_item_cnt),
	message_cb(NULL), list_layout(0)
{
  *status = 1;
}

//
//  Delete a nav context
//
TblNav::~TblNav()
{
}

TblNavBrow::~TblNavBrow()
{
  free_pixmaps();
}

int TblNav::is_authorized( unsigned int access, int msg)
{
  return (is_authorized_cb)( parent_ctx, access, msg);
}


//
// Callbacks from brow
//
static int tblnav_brow_cb( FlowCtx *ctx, flow_tEvent event)
{
  TblNav		*tblnav;
  ItemBase 		*item;

  if ( event->event == flow_eEvent_ObjectDeleted) {
    brow_GetUserData( event->object.object, (void **)&item);
    delete item;
    return 1;
  }

  brow_GetCtxUserData( (BrowCtx *)ctx, (void **) &tblnav);

  if ( !tblnav->is_authorized())
    return 1;

  tblnav->message( ' ', "");
  switch ( event->event) {
  case flow_eEvent_Key_PageDown: {
    brow_Page( tblnav->brow->ctx, 0.8);
    break;
  }
  case flow_eEvent_Key_PageUp: {
    brow_Page( tblnav->brow->ctx, -0.8);
    break;
  }
  case flow_eEvent_ScrollDown: {
    brow_Page( tblnav->brow->ctx, 0.1);
    break;
  }
  case flow_eEvent_ScrollUp: {
    brow_Page( tblnav->brow->ctx, -0.1);
    break;
  }
  case flow_eEvent_Key_Up: {
    brow_tNode	*node_list;
    int		node_count;
    brow_tObject	object;
    int		sts;
      
    brow_GetSelectedNodes( tblnav->brow->ctx, &node_list, &node_count);
    if ( !node_count) {
      sts = brow_GetLastVisible( tblnav->brow->ctx, &object);
      if ( EVEN(sts)) return 1;
    }
    else {
      if ( !brow_IsVisible( tblnav->brow->ctx, node_list[0], flow_eVisible_Partial)) {
	sts = brow_GetLastVisible( tblnav->brow->ctx, &object);
	if ( EVEN(sts)) return 1;
      }
      else {
	sts = brow_GetPrevious( tblnav->brow->ctx, node_list[0], &object);
	if ( EVEN(sts)) {
	  if ( node_count)
	    free( node_list);
	  return 1;
	}
      }
    }
    brow_SelectClear( tblnav->brow->ctx);
    brow_SetInverse( object, 1);
    brow_SelectInsert( tblnav->brow->ctx, object);
    if ( !brow_IsVisible( tblnav->brow->ctx, object, flow_eVisible_Full))
      brow_CenterObject( tblnav->brow->ctx, object, 0.25);
    if ( node_count)
      free( node_list);
    break;
  }
  case flow_eEvent_Key_Down: {
    brow_tNode	*node_list;
    int		node_count;
    brow_tObject	object;
    int		sts;

    brow_GetSelectedNodes( tblnav->brow->ctx, &node_list, &node_count);
    if ( !node_count) {
      sts = brow_GetFirstVisible( tblnav->brow->ctx, &object);
      if ( EVEN(sts)) return 1;
    }
    else {
      if ( !brow_IsVisible( tblnav->brow->ctx, node_list[0], flow_eVisible_Partial)) {
	sts = brow_GetFirstVisible( tblnav->brow->ctx, &object);
	if ( EVEN(sts)) return 1;
      }
      else {
	sts = brow_GetNext( tblnav->brow->ctx, node_list[0], &object);
	if ( EVEN(sts)) {
	  if ( node_count)
	    free( node_list);
	  return 1;
	}
      }
    }
    brow_SelectClear( tblnav->brow->ctx);
    brow_SetInverse( object, 1);
    brow_SelectInsert( tblnav->brow->ctx, object);
    if ( !brow_IsVisible( tblnav->brow->ctx, object, flow_eVisible_Full))
      brow_CenterObject( tblnav->brow->ctx, object, 0.75);
    if ( node_count)
      free( node_list);
    break;
  }
  case flow_eEvent_SelectClear:
    brow_ResetSelectInverse( tblnav->brow->ctx);
    break;
  case flow_eEvent_MB1Click: {
    // Select
    double ll_x, ll_y, ur_x, ur_y;
    int sts;

    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      brow_MeasureNode( event->object.object, &ll_x, &ll_y,
			&ur_x, &ur_y);
      if ( event->object.x < ll_x + 1.0) {
	// Simulate doubleclick
	flow_tEvent doubleclick_event;

	doubleclick_event = (flow_tEvent) calloc( 1, sizeof(*doubleclick_event));
	memcpy( doubleclick_event, event, sizeof(*doubleclick_event));
	doubleclick_event->event = flow_eEvent_MB1DoubleClick;
	sts = tblnav_brow_cb( ctx, doubleclick_event);
	free( (char *) doubleclick_event);
	return sts;
      }

      if ( brow_FindSelectedObject( tblnav->brow->ctx, event->object.object)) {
	brow_SelectClear( tblnav->brow->ctx);
      }
      else {
	brow_SelectClear( tblnav->brow->ctx);
	brow_SetInverse( event->object.object, 1);
	brow_SelectInsert( tblnav->brow->ctx, event->object.object);
      }
      break;
    default:
      brow_SelectClear( tblnav->brow->ctx);
    }
    break;
  }
  case flow_eEvent_Key_Left: {
    brow_tNode	*node_list;
    int		node_count;
    brow_tObject	object;
    int		sts;
    
    brow_GetSelectedNodes( tblnav->brow->ctx, &node_list, &node_count);
    if ( !node_count)
      return 1;

    if ( brow_IsOpen( node_list[0]))
      // Close this node
      object = node_list[0];
    else {
      // Close parent
      sts = brow_GetParent( tblnav->brow->ctx, node_list[0], &object);
      if ( EVEN(sts)) {
	free( node_list);
	return 1;
      }
    }
    brow_GetUserData( object, (void **)&item);
    item->close( tblnav, 0, 0);

    brow_SelectClear( tblnav->brow->ctx);
    brow_SetInverse( object, 1);
    brow_SelectInsert( tblnav->brow->ctx, object);
    if ( !brow_IsVisible( tblnav->brow->ctx, object, flow_eVisible_Full))
      brow_CenterObject( tblnav->brow->ctx, object, 0.25);
    free( node_list);
    break;
  }
  case flow_eEvent_Key_Right: {
    brow_tNode	*node_list;
    int		node_count;

    brow_GetSelectedNodes( tblnav->brow->ctx, &node_list, &node_count);
    if ( !node_count)
      return 1;

    brow_GetUserData( node_list[0], (void **)&item);
    switch( item->type) {
    case tblnav_eItemType_Local:
    case tblnav_eItemType_TreeLocal:
      ((ItemLocal *)item)->open_attributes( tblnav, 0, 0);
      break;
    case tblnav_eItemType_TreeNode:
      ((ItemTreeNode *)item)->open_children( tblnav, 0, 0);
      break;
    default:
      ;
    }
  }
  case flow_eEvent_MB1DoubleClick:
    switch ( event->object.object_type) {
    case flow_eObjectType_Node:
      brow_GetUserData( event->object.object, (void **)&item);
      switch( item->type) {
      case tblnav_eItemType_Local: 
      case tblnav_eItemType_TreeLocal: 
	((ItemLocal *)item)->open_attributes( tblnav,
					      event->object.x, event->object.y);
	break;
      case tblnav_eItemType_TreeNode: 
	((ItemTreeNode *)item)->open_children( tblnav,
						 event->object.x, event->object.y);
	break;
      default:
	;
      }
      break;
    default:
      ;
    }
    break;
  default:
    ;
  }
  return 1;
}


//
// Create nodeclasses
//
void TblNavBrow::create_nodeclasses()
{
  allocate_pixmaps();

  // Create common-class

  brow_CreateNodeClass( ctx, "NavigatorDefault", 
		flow_eNodeGroup_Common, &nc_object);
  brow_AddFrame( nc_object, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_object, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnotPixmap( nc_object, 1, 1.1, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_object, 2, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_object, 12, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);
  brow_AddAnnot( nc_object, 15, 0.6, 2,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

  // Create attribute nodeclass

  brow_CreateNodeClass( ctx, "NavigatorAttr", 
		flow_eNodeGroup_Common, &nc_attr);
  brow_AddFrame( nc_attr, 0, 0, 20, 0.8, flow_eDrawType_Line, -1, 1);
  brow_AddAnnotPixmap( nc_attr, 0, 0.2, 0.1, flow_eDrawType_Line, 2, 0);
  brow_AddAnnot( nc_attr, 2, 0.6, 0,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		0);
  brow_AddAnnot( nc_attr, 8, 0.6, 1,
		flow_eDrawType_TextHelveticaBold, 2, flow_eAnnotType_OneLine, 
		1);

}

void TblNav::show_list()
{
  list_layout = 1;
  brow_DeleteAll( brow->ctx);
  create_items();
}

void TblNav::show_tree()
{
  list_layout = 0;
  brow_DeleteAll( brow->ctx);
  create_items();
}

int TblNav::create_items()
{
  int i;

  brow_SetNodraw( brow->ctx);

  if ( list_layout) {
    for ( i = 0; i < item_cnt; i++) {
      new ItemLocal( this, &itemlist[i], NULL, flow_eDest_IntoLast);    
    }
  }
  else {
    for ( int idx = 1; idx; idx = tree[idx].fws) {
      if ( tree[idx].item)
	new ItemTreeLocal( this, tree[idx].item, idx, NULL, flow_eDest_IntoLast);
      else
	new ItemTreeNode( this, tree[idx].sname, idx, NULL, flow_eDest_IntoLast);
    }
  }
  brow_ResetNodraw( brow->ctx);
  brow_Redraw( brow->ctx, 0);
  return TBLNAV__SUCCESS;
}

void TblNavBrow::brow_setup()
{
  brow_sAttributes brow_attr;
  unsigned long mask;

  mask = 0;
  mask |= brow_eAttr_indentation;
  brow_attr.indentation = 0.5;
  mask |= brow_eAttr_annotation_space;
  brow_attr.annotation_space = 0.5;
  brow_SetAttributes( ctx, &brow_attr, mask); 
  brow_SetCtxUserData( ctx, tblnav);

  brow_EnableEvent( ctx, flow_eEvent_MB1Click, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_MB1DoubleClick, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_SelectClear, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ObjectDeleted, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Up, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Down, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Right, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_Left, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PF3, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Radiobutton, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PageUp, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_Key_PageDown, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ScrollUp, flow_eEventType_CallBack, 
	tblnav_brow_cb);
  brow_EnableEvent( ctx, flow_eEvent_ScrollDown, flow_eEventType_CallBack, 
	tblnav_brow_cb);
}

//
// Backcall routine called at creation of the brow widget
// Enable event, create nodeclasses and insert the root objects.
//
int TblNav::init_brow_cb( FlowCtx *fctx, void *client_data)
{
  TblNav *tblnav = (TblNav *) client_data;
  BrowCtx *ctx = (BrowCtx *)fctx;

  tblnav->brow = new TblNavBrow( ctx, (void *)tblnav);

  tblnav->brow->brow_setup();
  tblnav->brow->create_nodeclasses();

  tblnav->build_tree();

  // Create the root items
  tblnav->create_items();

  //sts = brow_TraceInit( ctx, tblnav_trace_connect_bc, 
  //		tblnav_trace_disconnect_bc, tblnav_trace_scan_bc);
  //tblnav->trace_started = 1;

  //tblnav->trace_start();

  return 1;
}


int TblNav::get_select( sevcli_sHistItem **hi)
{
  brow_tNode	*node_list;
  int		node_count;
  ItemLocal 		*item;
      
  brow_GetSelectedNodes( brow->ctx, &node_list, &node_count);

  if ( !node_count)
    return 0;

  brow_GetUserData( node_list[0], (void **)&item);
  switch ( item->type) {
  case tblnav_eItemType_Local:
  case tblnav_eItemType_TreeLocal:
    *hi = &item->item;
    return 1;
  default: ;
  }
  return 0;
}
  

//
//  Get zoom
//
void TblNav::get_zoom( double *zoom_factor)
{
  brow_GetZoom( brow->ctx, zoom_factor);
}

//
//  Zoom
//
void TblNav::zoom( double zoom_factor)
{
  brow_Zoom( brow->ctx, zoom_factor);
}

//
//  Return to base zoom factor
//
void TblNav::unzoom()
{
  brow_UnZoom( brow->ctx);
}

void TblNav::delete_item( sevcli_sHistItem *hi)
{
  brow_tNode	*node_list;
  int		node_count;
  ItemBase 	*baseitem;
      
  brow_GetObjectList( brow->ctx, &node_list, &node_count);
  // Delete in tree
  for ( int i = 1; i < (int)tree.size(); i++) {
    if ( tree[i].deleted)
      continue;
    if ( tree[i].item &&
	 cdh_ObjidIsEqual( tree[i].item->oid, hi->oid) &&
	 strcmp( tree[i].item->attr[0].aname, hi->attr[0].aname) == 0) {
      tree[i].deleted = 1;
    }
  }

  // Delete node if visible
  brow_GetObjectList( brow->ctx, &node_list, &node_count);

  int found = 0;
  for ( int i = 0; i < node_count; i++) {
    brow_GetUserData( node_list[i], (void **)&baseitem);
    switch ( baseitem->type) {
    case tblnav_eItemType_Local:
    case tblnav_eItemType_TreeLocal: {
      ItemLocal *item = (ItemLocal *)baseitem;
      if ( cdh_ObjidIsEqual( hi->oid, item->item.oid) &&
	   strcmp( hi->attr[0].aname, item->item.attr[0].aname) == 0) {
	brow_DeleteNode( brow->ctx, item->node);
	found = 1;
      }
      break;
    }
    default: ;
    }
    if ( found)
      break;
  }
}

ItemLocal::ItemLocal( TblNav *tblnav, sevcli_sHistItem *xitem,
		      brow_tNode dest, flow_eDest dest_code) : 
  ItemBase( tblnav_eItemType_Local), item(*xitem)
{
  type = tblnav_eItemType_Local;
  pwr_tAName aname;
  
  brow_CreateNode( tblnav->brow->ctx, "LocalItem", tblnav->brow->nc_object, 
		   dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, tblnav->brow->pixmap_leaf);

  strcpy( aname, item.oname);
  strcat( aname, ".");
  strcat( aname, item.attr[0].aname);
  brow_SetAnnotation( node, 0, aname, strlen(aname));
  brow_SetAnnotation( node, 1, item.description, strlen(item.description));
}

int ItemLocal::open_attributes( TblNav *tblnav, double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node)) {
    // Close
    brow_SetNodraw( tblnav->brow->ctx);
    brow_CloseNode( tblnav->brow->ctx, node);
    brow_SetAnnotPixmap( node, 0, tblnav->brow->pixmap_leaf);
    brow_ResetOpen( node, tblnav_mOpen_All);
    brow_ResetNodraw( tblnav->brow->ctx);
    brow_Redraw( tblnav->brow->ctx, node_y);
  }
  else {
    char value[256];
    char txt[80];

    brow_SetNodraw( tblnav->brow->ctx);

    
    new ItemLocalAttr( tblnav, "Description", item.description, node, flow_eDest_IntoLast);

    strcpy( value, "");
    cdh_ObjidToString( value, item.oid, 1);
    new ItemLocalAttr( tblnav, "Oid", value, node, flow_eDest_IntoLast);

    new ItemLocalAttr( tblnav, "Object", item.oname, node, flow_eDest_IntoLast);

    time_DtoAscii( &item.storagetime, 0, value, sizeof(value));
    new ItemLocalAttr( tblnav, "StorageTime", value, node, flow_eDest_IntoLast);

    time_AtoAscii( &item.creatime, time_eFormat_DateAndTime, value, sizeof(value));
    new ItemLocalAttr( tblnav, "CreationTime", value, node, flow_eDest_IntoLast);

    time_AtoAscii( &item.modtime, time_eFormat_DateAndTime, value, sizeof(value));
    new ItemLocalAttr( tblnav, "ModificationTime", value, node, flow_eDest_IntoLast);

    sprintf( value, "%f", item.scantime);
    new ItemLocalAttr( tblnav, "ScanTime", value, node, flow_eDest_IntoLast);

    strcpy( value, "");
    if ( item.options & pwr_mSevOptionsMask_PosixTime)
      strcat( value, " PosixTime");
    if ( item.options & pwr_mSevOptionsMask_HighTimeResolution)
      strcat( value, " HighTimeResolution");
    if ( item.options & pwr_mSevOptionsMask_ReadOptimized)
      strcat( value, " ReadOptimized");
    if ( item.options & pwr_mSevOptionsMask_UseDeadBand)
      strcat( value, " UseDeadBand");
    new ItemLocalAttr( tblnav, "Options", value, node, flow_eDest_IntoLast);

    sprintf( value, "%f", item.deadband);
    new ItemLocalAttr( tblnav, "Deadband", value, node, flow_eDest_IntoLast);

    for ( int i = 0; i < 1; i++) {
      sprintf( txt, "Attr[%d].Name", i);
      new ItemLocalAttr( tblnav, txt, item.attr[0].aname, node, flow_eDest_IntoLast);

      switch ( item.attr[0].type) {
      case pwr_eType_Int64: strcpy( value, "Int64"); break;
      case pwr_eType_Int32: strcpy( value, "Int32"); break;
      case pwr_eType_Int16: strcpy( value, "Int16"); break;
      case pwr_eType_Int8: strcpy( value, "Int8"); break;
      case pwr_eType_UInt64: strcpy( value, "UInt64"); break;
      case pwr_eType_UInt32: strcpy( value, "UInt32"); break;
      case pwr_eType_UInt16: strcpy( value, "UInt16"); break;
      case pwr_eType_UInt8: strcpy( value, "UInt8"); break;
      case pwr_eType_Boolean: strcpy( value, "Boolean"); break;
      case pwr_eType_Char: strcpy( value, "Char"); break;
      case pwr_eType_Float32: strcpy( value, "Float32"); break;
      case pwr_eType_Float64: strcpy( value, "Float64"); break;
      case pwr_eType_String: strcpy( value, "String"); break;
      case pwr_eType_Time: strcpy( value, "Time"); break;
      case pwr_eType_DeltaTime: strcpy( value, "DeltaTime"); break;
      default: strcpy( value, "Unknown");
      }
      sprintf( txt, "Attr[%d].DataType", i);
      new ItemLocalAttr( tblnav, txt, value, node, flow_eDest_IntoLast);

      sprintf( txt, "Attr[%d].DataSize", i);
      sprintf( value, "%d", item.attr[0].size);
      new ItemLocalAttr( tblnav, txt, value, node, flow_eDest_IntoLast);
    
      sprintf( txt, "Attr[%d].Unit", i);
      new ItemLocalAttr( tblnav, txt, item.attr[0].unit, node, flow_eDest_IntoLast);
    }
    brow_SetOpen( node, tblnav_mOpen_Attributes);
    brow_ResetNodraw( tblnav->brow->ctx);
    brow_Redraw( tblnav->brow->ctx, node_y);
  }
  return 1;
}

int ItemLocal::close( TblNav *tblnav, double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node)) {
    // Close
    brow_SetNodraw( tblnav->brow->ctx);
    brow_CloseNode( tblnav->brow->ctx, node);
    brow_SetAnnotPixmap( node, 0, tblnav->brow->pixmap_leaf);
    brow_ResetOpen( node, tblnav_mOpen_All);
    brow_ResetNodraw( tblnav->brow->ctx);
    brow_Redraw( tblnav->brow->ctx, node_y);
  }
  return 1;
}

ItemLocalAttr::ItemLocalAttr( TblNav *tblnav, char *name, char *value,
			      brow_tNode dest, flow_eDest dest_code) :
  ItemBase( tblnav_eItemType_LocalAttr)
{
  brow_CreateNode( tblnav->brow->ctx, "LocalItemAttr", tblnav->brow->nc_attr, 
		   dest, dest_code, (void *) this, 1, &node);

  brow_SetAnnotPixmap( node, 0, tblnav->brow->pixmap_attr);

  brow_SetAnnotation( node, 0, name, strlen(name));
  brow_SetAnnotation( node, 1, value, strlen(value));
}

ItemTreeLocal::ItemTreeLocal( TblNav *tblnav, sevcli_sHistItem *xitem, int index,
			      brow_tNode dest, flow_eDest dest_code) : 
  ItemLocal(tblnav, xitem, dest, dest_code), idx(index)
{
  pwr_tAName aname;
  char *s;

  type = tblnav_eItemType_TreeLocal;

  if ((s = strrchr( item.oname, '-')))  
    strcpy( aname, s+1);
  else
    strcpy( aname, item.oname);
  strcat( aname, ".");
  strcat( aname, item.attr[0].aname);
  brow_SetAnnotation( node, 0, aname, strlen(aname));
}

ItemTreeNode::ItemTreeNode( TblNav *tblnav, char *name, int index,
			    brow_tNode dest, flow_eDest dest_code) :
  ItemBase(tblnav_eItemType_TreeNode), idx(index)
{
  brow_CreateNode( tblnav->brow->ctx, "TreeNode", tblnav->brow->nc_attr, 
		   dest, dest_code, (void *) this, 1, &node);

  if ( tblnav->tree[idx].fch)
    brow_SetAnnotPixmap( node, 0, tblnav->brow->pixmap_map);
  else
    brow_SetAnnotPixmap( node, 0, tblnav->brow->pixmap_leaf);

  brow_SetAnnotation( node, 0, name, strlen(name));
}

int ItemTreeNode::open_children( TblNav *tblnav, double x, double y)
{
  double	node_x, node_y;

  brow_GetNodePosition( node, &node_x, &node_y);

  if ( brow_IsOpen( node) & tblnav_mOpen_Children) {
    // Attributes is open, close
    brow_SetNodraw( tblnav->brow->ctx);
    brow_CloseNode( tblnav->brow->ctx, node);
    brow_ResetOpen( node, tblnav_mOpen_Children);
    brow_RemoveAnnotPixmap( node, 1);
    brow_ResetNodraw( tblnav->brow->ctx);
    brow_Redraw( tblnav->brow->ctx, node_y);
  }
  else {

    brow_SetNodraw( tblnav->brow->ctx);
    for ( int i = tblnav->tree[idx].fch; i; i = tblnav->tree[i].fws) {
      if ( tblnav->tree[i].deleted)
	continue;

      if ( tblnav->tree[i].item)
	  new ItemTreeLocal( tblnav, tblnav->tree[i].item, i, node, flow_eDest_IntoLast);
      else
	  new ItemTreeNode( tblnav, tblnav->tree[i].sname, i, node, flow_eDest_IntoLast);
    }
    brow_SetOpen( node, tblnav_mOpen_Children);
    brow_SetAnnotPixmap( node, 1, tblnav->brow->pixmap_openmap);
    brow_ResetNodraw( tblnav->brow->ctx);
    brow_Redraw( tblnav->brow->ctx, node_y);
  }
  return 1;
}

int ItemTreeNode::close( TblNav *tblnav, double x, double y)
{
  double	node_x, node_y;

  if ( brow_IsOpen( node) & tblnav_mOpen_Children) {
    // Children is open, close
    brow_GetNodePosition( node, &node_x, &node_y);
    brow_SetNodraw( tblnav->brow->ctx);
    brow_CloseNode( tblnav->brow->ctx, node);
    brow_ResetOpen( node, tblnav_mOpen_All);
    brow_ResetNodraw( tblnav->brow->ctx);
    brow_Redraw( tblnav->brow->ctx, node_y);
  }
  return 1;
}

void TblNav::build_tree()
{
  pwr_tObjName name_array[24];
  pwr_tAName aname;
  int seg;

  for ( int i = 0; i < item_cnt; i++) {
    TblTreeNode n;

    strcpy( aname, itemlist[i].oname);
    strcat( aname, ".");
    strcat( aname, itemlist[i].attr[0].aname);

    seg = dcli_parse( aname, "-", "",
	     (char *) name_array, sizeof( name_array)/sizeof( name_array[0]), 
	     sizeof( name_array[0]), 0);

    if ( tree.size() == 0) {
      // First item
      TblTreeNode n0;
      strcpy( n0.sname, "nonode");
      tree.push_back(n0);

      for ( int j = 0; j < seg; j++) {
	TblTreeNode n;

	n.fth = j;
	tree[j].fch = j + 1;
	strcpy( n.sname, name_array[j]);
	if ( j == seg -1)
	  n.item = &itemlist[i];
	tree.push_back(n);
      }
    }
    else {
      int idx = 1;
      int last = idx;
      for ( int j = 0; j < seg; j++) {
	int found = 0;
	while ( idx) {
	  last = idx;
	  if ( strcmp( tree[idx].sname, name_array[j]) == 0) {
	    found = 1;
	    break;
	  }
	  idx = tree[idx].fws;
	}
	if ( !found) {
	  TblTreeNode n;
	  strcpy( n.sname, name_array[j]);
	  n.fth = tree[last].fth;
	  n.bws = last;	  
	  tree[last].fws = tree.size();
	  if ( j == seg - 1)
	    n.item = &itemlist[i];
	  tree.push_back(n);

	  for ( int k = j + 1; k < seg; k++) {
	    TblTreeNode n;
	    
	    n.fth = tree.size() - 1;
	    if ( tree[n.fth].fch == 0)
	      tree[n.fth].fch = tree.size();
	    strcpy( n.sname, name_array[k]);
	    if ( k == seg - 1)
	      n.item = &itemlist[i];
	    tree.push_back(n);
	  }
	  break;
	}
	if ( tree[idx].fch == 0) {
	  TblTreeNode n;
	  strcpy( n.sname, name_array[j]);
	  n.fth = tree[last].fth;
	  n.bws = last;	  
	  tree[last].fws = tree.size();
	  if ( j == seg - 1)
	    n.item = &itemlist[i];
	  tree.push_back(n);
	  for ( int k = j + 1; k < seg; k++) {
	    TblTreeNode n;
	    
	    n.fth = tree.size() - 1;
	    if ( tree[n.fth].fch == 0)
	      tree[n.fth].fch = tree.size();
	    strcpy( n.sname, name_array[k]);
	    if ( k == seg - 1)
	      n.item = &itemlist[i];
	    tree.push_back(n);
	  }
	  break;
	}
	idx = tree[idx].fch;
	last = idx;
      }
    }
  }
}