/*
 * ProviewR   Open Source Process Control.
 * Copyright (C) 2005-2019 SSAB EMEA AB.
 *
 * This file is part of ProviewR.
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
 * along with ProviewR. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking ProviewR statically or dynamically with other modules is
 * making a combined work based on ProviewR. Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * ProviewR give you permission to, from the build function in the
 * ProviewR Configurator, combine ProviewR with modules generated by the
 * ProviewR PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every
 * copy of the combined work is accompanied by a complete copy of
 * the source code of ProviewR (the version used to produce the
 * combined work), being distributed under the terms of the GNU
 * General Public License plus this exception.
 */

#include <math.h>

#include "glow_curvectx.h"
#include "glow_text.h"
#include "glow_conpoint.h"
#include "glow_con.h"
#include "glow_growtrend.h"
#include "glow_growaxis.h"
#include "glow_msg.h"

void CurveCtx::configure()
{
  //  GrowTrend *t1;
  // GrowAxis *a1;

  // t1 = new GrowTrend( this, "curve", 0, 0, 200, 30, glow_eDrawType_Line,
  //    1, glow_mDisplayLevel_1, 1, 1, glow_eDrawType_LineGray, 0);
  // t1->horizontal_lines = 9;
  // t1->vertical_lines = 29;
  // for ( int i = 0; i < 100; i++)
  //   t1->add_value( 50 * sin( i * 3.14 / 20) + 50, 0);
  // a.insert( t1);

  // a1 = new GrowAxis( this, "curve", 0, 30, 200, 32);
  // a1->text_size = 5;
  // a1->max_value = 300;
  // a1->min_value = 0;
  // a1->lines = 151;
  // a1->longquotient = 5;
  // a1->valuequotient = 10;
  // a1->trf.rotation = 270;
  // a1->configure();
  // a.insert( a1);
}

void CurveCtx::get_zoom(double* factor_x, double* factor_y)
{
  *factor_x = mw.zoom_factor_x;
  *factor_y = mw.zoom_factor_y;
}

void CurveCtx::zoom(double factor)
{
  if (fabs(factor) < DBL_EPSILON)
    return;

  mw.zoom_factor_x *= factor;
  mw.offset_x = int(
      (mw.offset_x - mw.window_width / 2.0 * (1.0 / factor - 1)) * factor);
  a.zoom();
  nav_zoom();
  change_scrollbar();
}

void CurveCtx::adjust_layout()
{
  int width, height;

  gdraw->get_window_size(mw.window, &width, &height);
  if ((mw.window_height != height && !feq(y_high, y_low)) || !layout_adjusted) {
    mw.zoom_factor_y = height / (y_high - y_low);
    if (!layout_adjusted && initial_position == glow_eDirection_Right) {
      mw.offset_x = int(x_right * mw.zoom_factor_x) - width;
    }
    set_dirty();
    layout_adjusted = 1;
  }
}

void CurveCtx::nav_zoom()
{
  if (nodraw)
    return;
  if (a.size() > 0) {
    set_dirty();
    get_borders();
    if (feq(x_right, x_left) || feq(y_high, y_low))
      return;
    navw.zoom_factor_x = navw.window_width / (x_right - x_left);
    navw.zoom_factor_y = navw.window_height / (y_high - y_low);
    navw.offset_x = int(x_left * navw.zoom_factor_x);
    navw.offset_y = int(y_low * navw.zoom_factor_y);
    a.nav_zoom();
  }
}

void CurveCtx::get_prefered_zoom_y(int height, double* factor_y)
{
  *factor_y = height / (y_high - y_low);
}

void CurveCtx::scroll(double value)
{
  int x_pix;

  if (value < 0
      && mw.offset_x + mw.window_width >= int(x_right * mw.zoom_factor_x))
    return;
  else if (value > 0 && mw.offset_x <= int(x_left * mw.zoom_factor_x))
    return;

  x_pix = int(mw.window_width * value);
  ((GlowCtx*)this)->scroll(x_pix, 0);
}

int CurveCtx::event_handler_nav(glow_eEvent event, int x, int y)
{
  switch (event) {
  case glow_eEvent_MB1Press:
    if (nav_rect_ll_x < x && x < nav_rect_ur_x && nav_rect_ll_y < y
        && y < nav_rect_ur_y) {
      nav_rect_movement_active = 1;
      nav_rect_move_last_x = x;
      nav_rect_move_last_y = y;
    }
    break;

  case glow_eEvent_MB2Press:
    if (nav_rect_ll_x < x && x < nav_rect_ur_x && nav_rect_ll_y < y
        && y < nav_rect_ur_y) {
      nav_rect_zoom_active = 1;
      nav_rect_move_last_x = x;
      nav_rect_move_last_y = y;
    }
    break;

  case glow_eEvent_CursorMotion:
    if (nav_rect_ll_x < x && x < nav_rect_ur_x && nav_rect_ll_y < y
        && y < nav_rect_ur_y) {
      if (!nav_rect_hot) {
        gdraw->set_cursor(navw.window, glow_eDrawCursor_CrossHair);
        nav_rect_hot = 1;
      }
    } else {
      if (nav_rect_hot) {
        gdraw->set_cursor(navw.window, glow_eDrawCursor_Normal);
        nav_rect_hot = 0;
      }
    }
    break;
  case glow_eEvent_Exposure:
    gdraw->get_window_size(navw.window, &navw.window_width, &navw.window_height);
    nav_zoom();
    break;
  case glow_eEvent_ButtonMotion:
    if (nav_rect_movement_active) {
      int delta_x = x - nav_rect_move_last_x;
      int delta_y = 0;
      nav_rect_ll_x += delta_x;
      nav_rect_ur_x += delta_x;
      nav_rect_ll_y += delta_y;
      nav_rect_ur_y += delta_y;
      nav_rect_move_last_x = x;
      nav_rect_move_last_y = y;

      int mainwind_delta_x = int(-mw.zoom_factor_x / navw.zoom_factor_x * delta_x);
      int mainwind_delta_y = int(-mw.zoom_factor_y / navw.zoom_factor_y * delta_y);
      mw.offset_x -= mainwind_delta_x;
      mw.offset_y -= mainwind_delta_y;
      if (ctx_type == glow_eCtxType_Grow) {
        ((GrowCtx*)this)->polyline_last_end_x += mainwind_delta_x;
        ((GrowCtx*)this)->polyline_last_end_y += mainwind_delta_y;
      }

      set_dirty();

      change_scrollbar();
    } else if (nav_rect_zoom_active) {
      double center_x = 0.5 * (nav_rect_ur_x + nav_rect_ll_x);
      double center_y = 0.5 * (nav_rect_ur_y + nav_rect_ll_y);
      double center_dist_last = sqrt(
          (nav_rect_move_last_x - center_x) * (nav_rect_move_last_x - center_x)
          + (nav_rect_move_last_y - center_y)
              * (nav_rect_move_last_y - center_y));
      double center_dist = sqrt(
          (x - center_x) * (x - center_x) + (y - center_y) * (y - center_y));
      if (center_dist < DBL_EPSILON)
        return 1;
      zoom(center_dist_last / center_dist);
      nav_rect_move_last_x = x;
      nav_rect_move_last_y = y;
    }
    break;
  case glow_eEvent_ButtonRelease:
    if (nav_rect_movement_active) {
      nav_rect_movement_active = 0;
      nav_zoom();
    }
    if (nav_rect_zoom_active) {
      nav_rect_zoom_active = 0;
    }
    break;
  default:;
  }
  return 1;
}

void curve_scroll_horizontal(CurveCtx* ctx, int value, int bottom)
{
  int x_pix = int(-value * ctx->scroll_size * ctx->mw.zoom_factor_x
      + (ctx->mw.offset_x - ctx->x_left * ctx->mw.zoom_factor_x));
  ((GlowCtx*)ctx)->scroll(x_pix, 0);
}

void curve_scroll_vertical(CurveCtx* ctx, int value, int bottom)
{
  int y_pix = int(-value * ctx->scroll_size * ctx->mw.zoom_factor_y
      + (ctx->mw.offset_y - ctx->y_low * ctx->mw.zoom_factor_y));
  // Correction for the bottom position
  if (bottom
      && (y_pix >= 0
             || ctx->mw.window_height + y_pix
                 < ctx->y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y))
    //        window_height >= (y_high - y_low) * zoom_factor_y)
    y_pix = int(ctx->mw.window_height + ctx->mw.offset_y
        - ctx->y_high * ctx->mw.zoom_factor_y);
  ((GlowCtx*)ctx)->scroll(0, y_pix);
}
