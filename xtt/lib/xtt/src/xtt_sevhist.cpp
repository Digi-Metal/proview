/* 
 * Proview   $Id: xtt_sevhist.cpp,v 1.2 2008-10-31 12:51:36 claes Exp $
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

#include "flow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "co_cdh.h"
#include "co_time.h"
#include "co_wow.h"
#include "rt_sev_msg.h"

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "glow_growctx.h"
#include "glow_growapi.h"
#include "glow_curvectx.h"
#include "glow_curveapi.h"

#include "xtt_xnav.h"
#include "xtt_sevhist.h"
#include "ge_curve.h"


XttSevHist::XttSevHist( void *parent_ctx,
		      const char *name,
		      pwr_tOid *xn_oid,
		      pwr_tOName *xn_aname,
		      sevcli_tCtx xn_scctx,
		      int *sts) :
  xnav(parent_ctx), gcd(0), curve(0), rows(0), vsize(0), timerid(0), close_cb(0), help_cb(0), 
  first_scan(1), scctx(xn_scctx), oid(xn_oid[0])
{
  strncpy( aname, xn_aname[0], sizeof(aname));
    
  get_data( sts, pwr_cNTime, pwr_cNTime);
  if ( EVEN(*sts)) return;

  cdh_StrncpyCutOff( title, name, sizeof(title), 1);
}

XttSevHist::~XttSevHist()
{
}



int XttSevHist::get_data( pwr_tStatus *sts, pwr_tTime from, pwr_tTime to)
{
  pwr_tTime *tbuf;
  void *vbuf;
  pwr_tDeltaTime trange;

  sevcli_get_itemdata( sts, scctx, oid, aname, from, to, 1000, &tbuf, &vbuf,
  		       &rows, &vtype, &vsize);
  if ( EVEN(*sts))
    return 0;

  // Create data for time axis
  gcd = new GeCurveData( curve_eDataType_DsTrend);

  gcd->data[0] = (double *) calloc( 1, 8 * rows);
  for ( int i = 0; i < rows; i++)
    gcd->data[0][i] = (double)tbuf[i].tv_sec + (double)1e-9 * tbuf[i].tv_nsec;

  strcpy( gcd->name[0], "Time");
  gcd->axis_type[0] = curve_eAxis_x;
  strcpy( gcd->format[0], "%10t");

  strcpy( gcd->name[1], aname);
  gcd->data[1] = (double *) calloc( 1, 8 * rows);

  for ( int i = 0; i < rows; i++) {
    switch ( vtype) {
    case pwr_eType_Int64:
      gcd->data[1][i] = ((pwr_tInt32 *)vbuf)[i];
      break;
    case pwr_eType_Int32:
      gcd->data[1][i] = ((pwr_tInt32 *)vbuf)[i];
      break;
    case pwr_eType_Int16:
      gcd->data[1][i] = ((pwr_tInt32 *)vbuf)[i];
      break;
    case pwr_eType_Int8:
      gcd->data[1][i] = ((pwr_tInt32 *)vbuf)[i];
      break;
    case pwr_eType_UInt64:
      gcd->data[1][i] = ((pwr_tUInt32 *)vbuf)[i];
      break;
    case pwr_eType_UInt32:
      gcd->data[1][i] = ((pwr_tUInt32 *)vbuf)[i];
      break;
    case pwr_eType_UInt16:
      gcd->data[1][i] = ((pwr_tUInt32 *)vbuf)[i];
      break;
    case pwr_eType_UInt8:
      gcd->data[1][i] = ((pwr_tUInt32 *)vbuf)[i];
      break;
    case pwr_eType_Float32:
      gcd->data[1][i] = ((pwr_tFloat32 *)vbuf)[i];
      break;
    case pwr_eType_Float64:
      gcd->data[1][i] = ((pwr_tFloat64 *)vbuf)[i];
      break;
    case pwr_eType_Boolean:
      gcd->data[1][i] = ((pwr_tBoolean *)vbuf)[i];
      break;
    default: 
      *sts = SEV__CURVETYPE;
      return 0;
    }
  }
  free( tbuf);
  free( vbuf);

  gcd->axis_type[1] = curve_eAxis_y;

  gcd->cols = 1 + 1;
  gcd->rows = rows;

  gcd->get_borders();
  gcd->get_default_axis();

  if ( to.tv_sec != 0 && from.tv_sec != 0) {
    time_Adiff( &trange, &to, &from);
    if ( time_DToFloat( 0, &trange) < 600)
      strcpy( gcd->format[0], "%10t");
    else
      strcpy( gcd->format[0], "%11t");
  }
  else
    strcpy( gcd->format[0], "%11t");
    

  gcd->select_color( 0);

  if ( curve) {
    curve->set_curvedata( gcd);  // This will free the old gcd 
    curve->configure_curves();
    curve->configure_axes();
    curve->redraw();
  }
  *sts = SEV__SUCCESS;
  return 1;
}

void XttSevHist::pop()
{
  curve->pop();
}

void XttSevHist::sevhist_close_cb( void *ctx)
{
  XttSevHist *sevhist = (XttSevHist *) ctx;

  if ( sevhist->close_cb)
    (sevhist->close_cb)( sevhist->xnav, sevhist);
  else
    delete sevhist;
}

void XttSevHist::sevhist_higher_res_cb( void *ctx)
{
  XttSevHist *sevhist = (XttSevHist *) ctx;
  double ll_x, ll_y, ur_x, ur_y;
  pwr_tTime t_low, t_high;
  pwr_tStatus sts;

  sevhist->curve->measure_window( &ll_x, &ll_y, &ur_x, &ur_y);

  t_low.tv_sec = int( sevhist->gcd->min_value_axis[0] + 
		      ll_x / 200 * (sevhist->gcd->max_value_axis[0] - sevhist->gcd->min_value_axis[0]));
  t_low.tv_nsec = 0;
  t_high.tv_sec = int( sevhist->gcd->min_value_axis[0] +
		       ur_x / 200 * (sevhist->gcd->max_value_axis[0] - sevhist->gcd->min_value_axis[0]));
  t_high.tv_nsec = 0;

  {
    char s1[40], s2[40];
    time_AtoAscii( &t_low, time_eFormat_NumDateAndTime, s1, sizeof(s1));
    time_AtoAscii( &t_high, time_eFormat_NumDateAndTime, s2, sizeof(s2));

    printf( "Low: %s, High: %s\n", s1, s2);
  }

  sevhist->get_data( &sts, t_low, t_high);
}

void XttSevHist::sevhist_lower_res_cb( void *ctx)
{
  XttSevHist *sevhist = (XttSevHist *) ctx;
  double ll_x, ll_y, ur_x, ur_y;
  pwr_tTime t_low, t_high;
  pwr_tStatus sts;

  sevhist->curve->measure_window( &ll_x, &ll_y, &ur_x, &ur_y);

  t_low.tv_sec = int( sevhist->gcd->min_value_axis[0] -  
		      5 * (sevhist->gcd->max_value_axis[0] - sevhist->gcd->min_value_axis[0]));
  t_low.tv_nsec = 0;
  t_high.tv_sec = int( sevhist->gcd->max_value_axis[0] +
		      5 * (sevhist->gcd->max_value_axis[0] - sevhist->gcd->min_value_axis[0]));
  t_high.tv_nsec = 0;

  {
    char s1[40], s2[40];
    time_AtoAscii( &t_low, time_eFormat_NumDateAndTime, s1, sizeof(s1));
    time_AtoAscii( &t_high, time_eFormat_NumDateAndTime, s2, sizeof(s2));

    printf( "Low: %s, High: %s\n", s1, s2);
  }
  sevhist->get_data( &sts, t_low, t_high);
}

void XttSevHist::sevhist_help_cb( void *ctx)
{
  XttSevHist *sevhist = (XttSevHist *) ctx;

  if ( sevhist->help_cb)
    (sevhist->help_cb)( sevhist->xnav, "opg_history");
}

void XttSevHist::sevhist_scan( void *data)
{
  XttSevHist *sevhist = (XttSevHist *) data;

  if ( sevhist->timerid)
    sevhist->timerid->add( 1000, sevhist_scan, sevhist);
}







