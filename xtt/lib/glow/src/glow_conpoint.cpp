/* 
 * Proview   $Id: glow_conpoint.cpp,v 1.4 2007-01-04 07:57:38 claes Exp $
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


#include <stdlib.h>
#include <iostream.h>
#include <math.h>

#include "glow_conpoint.h"
#include "glow_draw.h"

void GlowConPoint::zoom()
{
  p.zoom();
}

void GlowConPoint::nav_zoom()
{
  p.nav_zoom();
}

void GlowConPoint::print_zoom()
{
  p.print_zoom();
}

void GlowConPoint::save( ofstream& fp, glow_eSaveMode mode)
{
  fp <<	int(glow_eSave_ConPoint) << endl;
  fp <<	int(glow_eSave_ConPoint_number) << FSPACE << number << endl;
  fp <<	int(glow_eSave_ConPoint_direction) << FSPACE << int(direction) << endl;
  fp <<	int(glow_eSave_ConPoint_p) << endl;
  p.save( fp, mode);
  fp << int(glow_eSave_ConPoint_trace_attribute) << FSPACE << trace_attribute << endl;
  fp << int(glow_eSave_ConPoint_trace_attr_type) << FSPACE << int(trace_attr_type) << endl;
  fp << int(glow_eSave_ConPoint_trf) << endl;
  trf.save( fp, mode);
  fp <<	int(glow_eSave_End) << endl;
}

void GlowConPoint::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case glow_eSave_ConPoint: break;
      case glow_eSave_ConPoint_number: fp >> number; break;
      case glow_eSave_ConPoint_direction: fp >> tmp; direction = (glow_eDirection)tmp; break;
      case glow_eSave_ConPoint_p: p.open( fp); break;
      case glow_eSave_ConPoint_trace_attribute:
        fp.get();
        fp.getline( trace_attribute, sizeof(trace_attribute));
        break;
      case glow_eSave_ConPoint_trace_attr_type: fp >> tmp; trace_attr_type = (glow_eTraceType)tmp; break;
      case glow_eSave_ConPoint_trf: trf.open( fp); break;
      case glow_eSave_End: end_found = 1; break;
      default:
        cout << "GlowConPoint:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}


void GlowConPoint::traverse( int x, int y)
{
  p.traverse( x, y);
}

int GlowConPoint::event_handler( void *pos, glow_eEvent event, int x, int y,
	void *node)
{
  return 0;
}

void GlowConPoint::conpoint_select( void *pos, int x, int y, 
	double *distance, void **cp)
{
  int px, py;
  double dist;

  px = ((GlowPoint *)pos)->z_x - ctx->mw.offset_x + p.z_x;
  py = ((GlowPoint *)pos)->z_y - ctx->mw.offset_y + p.z_y;

  dist = sqrt( 1.0*(x-px)*(x-px) + 1.0*(y-py)*(y-py));
  if ( dist < *distance)
  {
    *distance = dist;
    *cp = (void *) this;
  }
}

void GlowConPoint::conpoint_select( GlowTransform *t, int x, int y, 
	double *distance, void **cp, int *pix_x, int *pix_y)
{
  int px, py;
  double dist;
  double x1, y1;

  x1 = trf.x( t, p.x, p.y);
  y1 = trf.y( t, p.x, p.y);
  px = int( x1 * ctx->mw.zoom_factor_x - ctx->mw.offset_x);
  py = int( y1 * ctx->mw.zoom_factor_y - ctx->mw.offset_y);

  dist = sqrt( 1.0*(x-px)*(x-px) + 1.0*(y-py)*(y-py));
  if ( dist < *distance)
  {
    *distance = dist;
    *cp = (void *) this;
    *pix_x = px;
    *pix_y = py;
  }
}

int GlowConPoint::get_conpoint( int num, double *x, double *y,
	glow_eDirection *dir)
{
  if ( number == num)
  {
    *x = p.x;
    *y = p.y;
    *dir = direction;
    return 1;
  }
  return 0;
}

int GlowConPoint::get_conpoint( GlowTransform *t, int num, bool flip_horizontal,
				bool flip_vertical, double *x, double *y,
	glow_eDirection *dir)
{
  double rotation;

  if ( number == num)
  {
    *x = trf.x( t, p.x, p.y);
    *y = trf.y( t, p.x, p.y);
    if ( direction == glow_eDirection_Center)
    {
      *dir = direction;
      return 1;
    }
    rotation = (trf.rot( t) / 360 - floor( trf.rot( t) / 360)) * 360;
    if ( 45 >= rotation || rotation > 315)
    {
      *dir = direction;
    }
    else if ( 45 < rotation && rotation <= 135)  
    {
      switch ( direction) {
        case glow_eDirection_Right: 	*dir = glow_eDirection_Up; break;
        case glow_eDirection_Up: 	*dir = glow_eDirection_Left; break;
        case glow_eDirection_Left: 	*dir = glow_eDirection_Down; break;
        case glow_eDirection_Down: 	*dir = glow_eDirection_Right; break;
        default: ;
      }
    }
    else if ( 135 < rotation && rotation <= 225)  
    {
      switch ( direction) {
        case glow_eDirection_Right: 	*dir = glow_eDirection_Left; break;
        case glow_eDirection_Up: 	*dir = glow_eDirection_Down; break;
        case glow_eDirection_Left: 	*dir = glow_eDirection_Right; break;
        case glow_eDirection_Down: 	*dir = glow_eDirection_Up; break;
        default: ;
      }
    }
    else // if ( 225 < rotation && rotation <= 315)
    {
      switch ( direction) {
        case glow_eDirection_Right: 	*dir = glow_eDirection_Down; break;
        case glow_eDirection_Up: 	*dir = glow_eDirection_Right; break;
        case glow_eDirection_Left: 	*dir = glow_eDirection_Up; break;
        case glow_eDirection_Down: 	*dir = glow_eDirection_Left; break;
        default: ;
      }
    }
    
    if ( flip_horizontal) {
      switch ( direction) {
        case glow_eDirection_Up: 	*dir = glow_eDirection_Down; break;
        case glow_eDirection_Down: 	*dir = glow_eDirection_Up; break;
        default: ;
      }
    }
    if ( flip_vertical) {
      switch ( direction) {
        case glow_eDirection_Right: 	*dir = glow_eDirection_Left; break;
        case glow_eDirection_Left: 	*dir = glow_eDirection_Right; break;
        default: ;
      }
    }

    return 1;
  }
  return 0;
}

ostream& operator<< ( ostream& o, const GlowConPoint cp)
{
  o << '(' << cp.p.x << ',' << cp.p.y << ')'; 
  return o;
}

