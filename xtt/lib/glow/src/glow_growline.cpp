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
#include <stdlib.h>

#include <iostream>

#include "co_string.h"

#include "glow_growline.h"
#include "glow_draw.h"
#include "glow_grownode.h"
#include "glow_exportflow.h"

GrowLine::GrowLine(GrowCtx* glow_ctx, const char* name, double x1, double y1,
    double x2, double y2, glow_eDrawType d_type, int line_w, int fix_line_w,
    int nodraw)
    : GlowLine(glow_ctx, x1, y1, x2, y2, d_type, line_w, fix_line_w), hot(0),
      pzero(ctx), highlight(0), inverse(0), original_border_drawtype(d_type),
      user_data(NULL), dynamic(0), dynamicsize(0),
      line_type(glow_eLineType_Solid)
{
  strcpy(n_name, name);
  pzero.nav_zoom();
  strcpy(last_group, "");

  if (ctx->grid_on) {
    double x_grid, y_grid;

    ctx->find_grid(p1.x, p1.y, &x_grid, &y_grid);
    p1.posit(x_grid, y_grid);
    ctx->find_grid(p2.x, p2.y, &x_grid, &y_grid);
    p2.posit(x_grid, y_grid);
  }
  if (!nodraw)
    ctx->set_dirty();
  get_node_borders();
}

GrowLine::~GrowLine()
{
  ctx->object_deleted(this);

  ctx->set_dirty();
  if (hot)
    ctx->gdraw->set_cursor(ctx->mw.window, glow_eDrawCursor_Normal);
}

void GrowLine::move(double delta_x, double delta_y, int grid)
{
  if (grid) {
    double x_grid, y_grid;

    /* Move to closest grid point */
    ctx->find_grid(x_left + delta_x / ctx->mw.zoom_factor_x,
        y_low + delta_y / ctx->mw.zoom_factor_y, &x_grid, &y_grid);
    trf.move(x_grid - x_left, y_grid - y_low);
    get_node_borders();
  } else {
    double dx = delta_x / ctx->mw.zoom_factor_x;
    double dy = delta_y / ctx->mw.zoom_factor_y;
    trf.move(dx, dy);
    x_right += dx;
    x_left += dx;
    y_high += dy;
    y_low += dy;
  }
  ctx->set_dirty();
}

int GrowLine::event_handler(glow_eEvent event, double fx, double fy)
{
  // Convert from global to local coordinates
  glow_sPoint p = trf.reverse(fx, fy);
  glow_sPoint tmp2 = trf.reverse(0.05 * line_width, 0.05 * line_width);
  glow_sPoint tmp1 = trf.reverse(0, 0);
  double dx = fabs(tmp2.x - tmp1.x);
  double dy = fabs(tmp2.y - tmp1.y);

  if ((feq(p1.x, p2.x) && p1.y < p2.y && // Vertical
       fabs(p1.x - p.x) < dx && p1.y < p.y && p.y < p2.y)
      || (feq(p1.x, p2.x) && p1.y > p2.y && // Vertical
          fabs(p1.x - p.x) < dx && p2.y < p.y && p.y < p1.y)
      || (feq(p1.y, p2.y) && p1.x < p2.x && // Horizontal
          fabs(p1.y - p.y) < dy && p1.x < p.x && p.x < p2.x)
      || (feq(p1.y, p2.y) && p1.x > p2.x && // Horizontal
          fabs(p1.y - p.y) < dy && p2.x < p.x && p.x < p1.x)) {
    return 1;
  } else if (
      (!(feq(p1.x, p2.x) || feq(p1.y, p2.y)) && p1.x < p2.x && p1.x <= p.x &&
       p.x <= p2.x && fabs(p.y - (p2.y - p1.y) / (p2.x - p1.x) * p.x - p1.y +
                         (p2.y - p1.y) / (p2.x - p1.x) * p1.x) < dx) ||
      (!(feq(p1.x, p2.x) || feq(p1.y, p2.y)) && p1.x > p2.x && p2.x <= p.x &&
       p.x <= p1.x && fabs(p.y - (p2.y - p1.y) / (p2.x - p1.x) * p.x - p1.y +
                         (p2.y - p1.y) / (p2.x - p1.x) * p1.x) < dx)) {
    return 1;
  }
  return 0;
}

int GrowLine::event_handler(glow_eEvent event, int x, int y, double fx,
    double fy)
{
  int sts = 0;
  if (event == ctx->event_move_node) {
    sts = event_handler(event, fx, fy);
    if (sts) {
      /* Register node for potential movement */
      ctx->move_insert(this);
    }
    return sts;
  }
  switch (event) {
  case glow_eEvent_CursorMotion: {
    if (ctx->hot_mode == glow_eHotMode_TraceAction)
      sts = 0;
    else if (ctx->hot_found)
      sts = 0;
    else {
      sts = event_handler(event, fx, fy);
      if (sts)
        ctx->hot_found = 1;
    }
    if (sts && !hot
        && !(ctx->node_movement_active || ctx->node_movement_paste_active)) {
      ctx->gdraw->set_cursor(ctx->mw.window, glow_eDrawCursor_CrossHair);
      set_hot(1);
    }
    if (!sts && hot) {
      if (!ctx->hot_found)
        ctx->gdraw->set_cursor(ctx->mw.window, glow_eDrawCursor_Normal);
      set_hot(0);
    }
    break;
  }
  default:
    sts = event_handler(event, fx, fy);
  }
  if (sts)
    ctx->register_callback_object(glow_eObjectType_Node, this);
  return sts;
}

void GrowLine::save(std::ofstream& fp, glow_eSaveMode mode)
{
  fp << int(glow_eSave_GrowLine) << '\n';
  fp << int(glow_eSave_GrowLine_n_name) << FSPACE << n_name << '\n';
  fp << int(glow_eSave_GrowLine_x_right) << FSPACE << x_right << '\n';
  fp << int(glow_eSave_GrowLine_x_left) << FSPACE << x_left << '\n';
  fp << int(glow_eSave_GrowLine_y_high) << FSPACE << y_high << '\n';
  fp << int(glow_eSave_GrowLine_y_low) << FSPACE << y_low << '\n';
  fp << int(glow_eSave_GrowLine_original_border_drawtype) << FSPACE
     << int(original_border_drawtype) << '\n';
  fp << int(glow_eSave_GrowLine_line_type) << FSPACE << int(line_type) << '\n';
  fp << int(glow_eSave_GrowLine_line_part) << '\n';
  GlowLine::save(fp, mode);
  fp << int(glow_eSave_GrowLine_trf) << '\n';
  trf.save(fp, mode);
  fp << int(glow_eSave_End) << '\n';
}

void GrowLine::open(std::ifstream& fp)
{
  int type = 0;
  int end_found = 0;
  char dummy[40];
  int tmp;

  for (;;) {
    if (!fp.good()) {
      fp.clear();
      fp.getline(dummy, sizeof(dummy));
      printf("** Read error GrowLine: \"%d %s\"\n", type, dummy);
    }

    fp >> type;
    switch (type) {
    case glow_eSave_GrowLine:
      break;
    case glow_eSave_GrowLine_n_name:
      fp.get();
      fp.getline(n_name, sizeof(n_name));
      break;
    case glow_eSave_GrowLine_x_right:
      fp >> x_right;
      break;
    case glow_eSave_GrowLine_x_left:
      fp >> x_left;
      break;
    case glow_eSave_GrowLine_y_high:
      fp >> y_high;
      break;
    case glow_eSave_GrowLine_y_low:
      fp >> y_low;
      break;
    case glow_eSave_GrowLine_original_border_drawtype:
      fp >> tmp;
      original_border_drawtype = (glow_eDrawType)tmp;
      break;
    case glow_eSave_GrowLine_line_type:
      fp >> tmp;
      line_type = (glow_eLineType)tmp;
      break;
    case glow_eSave_GrowLine_line_part:
      GlowLine::open(fp);
      break;
    case glow_eSave_GrowLine_trf:
      trf.open(fp);
      break;
    case glow_eSave_End:
      end_found = 1;
      break;
    default:
      std::cout << "GrowLine:open syntax error\n";
      fp.getline(dummy, sizeof(dummy));
    }
    if (end_found)
      break;
  }
}

void GrowLine::draw(GlowWind* w, int ll_x, int ll_y, int ur_x, int ur_y)
{
  int tmp;

  if (ll_x > ur_x) {
    /* Shift */
    tmp = ll_x;
    ll_x = ur_x;
    ur_x = tmp;
  }
  if (ll_y > ur_y) {
    /* Shift */
    tmp = ll_y;
    ll_y = ur_y;
    ur_y = tmp;
  }

  if (x_right * w->zoom_factor_x - w->offset_x >= ll_x
      && x_left * w->zoom_factor_x - w->offset_x <= ur_x
      && y_high * w->zoom_factor_y - w->offset_y >= ll_y
      && y_low * w->zoom_factor_y - w->offset_y <= ur_y) {
    draw(w, (GlowTransform*)NULL, highlight, hot, NULL, NULL);
  }
}

void GrowLine::draw(GlowWind* w, int* ll_x, int* ll_y, int* ur_x, int* ur_y)
{
  int tmp;
  int obj_ur_x = int(x_right * w->zoom_factor_x) - w->offset_x;
  int obj_ll_x = int(x_left * w->zoom_factor_x) - w->offset_x;
  int obj_ur_y = int(y_high * w->zoom_factor_y) - w->offset_y;
  int obj_ll_y = int(y_low * w->zoom_factor_y) - w->offset_y;

  if (*ll_x > *ur_x) {
    /* Shift */
    tmp = *ll_x;
    *ll_x = *ur_x;
    *ur_x = tmp;
  }
  if (*ll_y > *ur_y) {
    /* Shift */
    tmp = *ll_y;
    *ll_y = *ur_y;
    *ur_y = tmp;
  }

  if (obj_ur_x >= *ll_x && obj_ll_x <= *ur_x && obj_ur_y >= *ll_y
      && obj_ll_y <= *ur_y) {
    draw(w, (GlowTransform*)NULL, highlight, hot, NULL, NULL);

    // Increase the redraw area
    if (obj_ur_x > *ur_x)
      *ur_x = obj_ur_x;
    if (obj_ur_y > *ur_y)
      *ur_y = obj_ur_y;
    if (obj_ll_x < *ll_x)
      *ll_x = obj_ll_x;
    if (obj_ll_y < *ll_y)
      *ll_y = obj_ll_y;
  }
}

void GrowLine::set_highlight(int on)
{
  if (highlight != on) {
    highlight = on;
    ctx->set_dirty();
  }
}

void GrowLine::get_borders(GlowTransform* t, double* x_right, double* x_left,
    double* y_high, double* y_low)
{
  Matrix tmp = t ? (*t * trf) : trf;
  glow_sPoint p1 = tmp * this->p1;
  glow_sPoint p2 = tmp * this->p2;

  *x_left = MIN(*x_left, MIN(p1.x, p2.x));
  *x_right = MAX(*x_right, MAX(p1.x, p2.x));
  *y_low = MIN(*y_low, MIN(p1.y, p2.y));
  *y_high = MAX(*y_high, MAX(p1.y, p2.y));
}

void GrowLine::select_region_insert(double ll_x, double ll_y, double ur_x,
    double ur_y, glow_eSelectPolicy select_policy)
{
  if (select_policy == glow_eSelectPolicy_Surround) {
    if (x_left > ll_x && x_right < ur_x && y_high < ur_y && y_low > ll_y)
      ctx->select_insert(this);
  } else {
    if (x_right > ll_x && x_left < ur_x && y_low < ur_y && y_high > ll_y)
      ctx->select_insert(this);
  }
}

void GrowLine::set_dynamic(char* code, int size)
{
  if (!dynamic) {
    dynamic = (char*)calloc(1, size + 1);
    dynamicsize = size + 1;
  } else if (dynamicsize < size + 1) {
    free(dynamic);
    dynamic = (char*)calloc(1, size + 1);
    dynamicsize = size + 1;
  }
  strncpy(dynamic, code, size + 1);
}

void GrowLine::exec_dynamic()
{
  if (dynamicsize && !streq(dynamic, ""))
    ctx->dynamic_cb(this, dynamic, glow_eDynamicType_Object);
}

void GrowLine::set_position(double x, double y)
{
  if (feq(trf.a13, x) && feq(trf.a23, y))
    return;

  trf.posit(x, y);
  get_node_borders();
  ctx->set_dirty();
}

void GrowLine::set_scale(
    double scale_x, double scale_y, double x0, double y0, glow_eScaleType type)
{
  if (trf.s.a11 && trf.s.a22
      && fabs(scale_x - trf.a11 / trf.s.a11) < FLT_EPSILON
      && fabs(scale_y - trf.a22 / trf.s.a22) < FLT_EPSILON)
    return;

  switch (type) {
  case glow_eScaleType_LowerLeft:
    x0 = x_left;
    y0 = y_low;
    break;
  case glow_eScaleType_LowerRight:
    x0 = x_right;
    y0 = y_low;
    break;
  case glow_eScaleType_UpperRight:
    x0 = x_right;
    y0 = y_high;
    break;
  case glow_eScaleType_UpperLeft:
    x0 = x_left;
    y0 = y_high;
    break;
  case glow_eScaleType_FixPoint:
    break;
  case glow_eScaleType_Center:
    x0 = (x_left + x_right) / 2;
    y0 = (y_low + y_high) / 2;
    break;
  default:;
  }

  double old_x_left = x_left, old_y_low = y_low;
  double old_x_right = x_right, old_y_high = y_high;
  trf.scale_from_stored(scale_x, scale_y, x0, y0);
  get_node_borders();

  switch (type) {
  case glow_eScaleType_LowerLeft:
    x_left = old_x_left;
    y_low = old_y_low;
    break;
  case glow_eScaleType_LowerRight:
    x_right = old_x_right;
    y_low = old_y_low;
    break;
  case glow_eScaleType_UpperRight:
    x_right = old_x_right;
    y_high = old_y_high;
    break;
  case glow_eScaleType_UpperLeft:
    x_left = old_x_left;
    y_high = old_y_high;
    break;
  case glow_eScaleType_FixPoint:
    break;
  case glow_eScaleType_Center:
    x0 = (x_left + x_right) / 2;
    y0 = (y_low + y_high) / 2;
    break;
  default:;
  }
  ctx->set_dirty();
}

void GrowLine::set_rotation(
    double angle, double x0, double y0, glow_eRotationPoint type)
{
  if (fabs(angle - trf.rotation + trf.s.rotation) < FLT_EPSILON)
    return;

  switch (type) {
  case glow_eRotationPoint_LowerLeft:
    x0 = x_left;
    y0 = y_low;
    break;
  case glow_eRotationPoint_LowerRight:
    x0 = x_right;
    y0 = y_low;
    break;
  case glow_eRotationPoint_UpperRight:
    x0 = x_right;
    y0 = y_high;
    break;
  case glow_eRotationPoint_UpperLeft:
    x0 = x_left;
    y0 = y_high;
    break;
  case glow_eRotationPoint_Center:
    x0 = (x_left + x_right) / 2;
    y0 = (y_high + y_low) / 2;
    break;
  default:;
  }

  trf.rotate_from_stored(angle, x0, y0);
  get_node_borders();
  ctx->set_dirty();
}

void GrowLine::draw(GlowWind* w, GlowTransform* t, int highlight, int hot,
    void* node, void* colornode)
{
  hot = (w == &ctx->navw) ? 0 : hot;
  if (hot && ctx->environment != glow_eEnv_Development
      && ctx->hot_indication != glow_eHotIndication_LineWidth)
    hot = 0;

  int idx;
  if (node && ((GrowNode*)node)->line_width)
    idx = int(
        w->zoom_factor_y / w->base_zoom_factor * ((GrowNode*)node)->line_width
        - 1);
  else
    idx = int(w->zoom_factor_y / w->base_zoom_factor * line_width - 1);
  idx += hot;
  idx = MAX(0, idx);
  idx = MIN(idx, DRAW_TYPE_SIZE - 1);

  Matrix tmp = t ? (*t * trf) : trf;
  glow_sPoint p1 = tmp * this->p1;
  glow_sPoint p2 = tmp * this->p2;

  p1.x = p1.x * w->zoom_factor_x - w->offset_x;
  p1.y = p1.y * w->zoom_factor_y - w->offset_y;
  p2.x = p2.x * w->zoom_factor_x - w->offset_x;
  p2.y = p2.y * w->zoom_factor_y - w->offset_y;

  if (feq(p1.x, p2.x) && feq(p1.y, p2.y))
    return;

  glow_eDrawType drawtype = ctx->get_drawtype(draw_type, glow_eDrawType_LineHighlight,
      highlight, (GrowNode*)colornode, 0);

  ctx->gdraw->line((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, drawtype, idx,
      0, line_type);
}

void GrowLine::erase(GlowWind* w, GlowTransform* t, int hot, void* node)
{
  hot = (w == &ctx->navw) ? 0 : hot;
  if (hot && ctx->environment != glow_eEnv_Development
      && ctx->hot_indication != glow_eHotIndication_LineWidth)
    hot = 0;

  int idx;
  if (node && ((GrowNode*)node)->line_width)
    idx = int(
        w->zoom_factor_y / w->base_zoom_factor * ((GrowNode*)node)->line_width
        - 1);
  else
    idx = int(w->zoom_factor_y / w->base_zoom_factor * line_width - 1);
  idx += hot;
  idx = MAX(0, idx);
  idx = MIN(idx, DRAW_TYPE_SIZE - 1);

  Matrix tmp = t ? (*t * trf) : trf;
  glow_sPoint p1 = tmp * this->p1;
  glow_sPoint p2 = tmp * this->p2;

  p1.x = p1.x * w->zoom_factor_x - w->offset_x;
  p1.y = p1.y * w->zoom_factor_y - w->offset_y;
  p2.x = p2.x * w->zoom_factor_x - w->offset_x;
  p2.y = p2.y * w->zoom_factor_y - w->offset_y;

  if (feq(p1.x, p2.x) && feq(p1.y, p2.y))
    return;

  ctx->gdraw->line((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y,
      glow_eDrawType_LineErase, idx);
}

void GrowLine::set_transform(GlowTransform* t)
{
  trf.set(*t * trf);
  get_node_borders();
}

void GrowLine::align(double x, double y, glow_eAlignDirection direction)
{
  double dx, dy;

  switch (direction) {
  case glow_eAlignDirection_CenterVert:
    dx = x - (x_right + x_left) / 2;
    dy = 0;
    break;
  case glow_eAlignDirection_CenterHoriz:
    dx = 0;
    dy = y - (y_high + y_low) / 2;
    break;
  case glow_eAlignDirection_CenterCenter:
    dx = x - (x_right + x_left) / 2;
    dy = y - (y_high + y_low) / 2;
    break;
  case glow_eAlignDirection_Right:
    dx = x - x_right;
    dy = 0;
    break;
  case glow_eAlignDirection_Left:
    dx = x - x_left;
    dy = 0;
    break;
  case glow_eAlignDirection_Up:
    dx = 0;
    dy = y - y_high;
    break;
  case glow_eAlignDirection_Down:
    dx = 0;
    dy = y - y_low;
    break;
  }
  trf.move(dx, dy);
  x_right += dx;
  x_left += dx;
  y_high += dy;
  y_low += dy;
  ctx->set_dirty();
}

void GrowLine::export_javabean(GlowTransform* t, void* node,
    glow_eExportPass pass, int* shape_cnt, int node_cnt, int in_nc,
    std::ofstream& fp)
{
  int idx;
  if (node && ((GrowNode*)node)->line_width)
    idx = int(ctx->mw.zoom_factor_y / ctx->mw.base_zoom_factor
            * ((GrowNode*)node)->line_width
        - 1);
  else
    idx = int(
        ctx->mw.zoom_factor_y / ctx->mw.base_zoom_factor * line_width - 1);
  idx += hot;
  idx = MAX(0, idx);
  idx = MIN(idx, DRAW_TYPE_SIZE - 1);

  Matrix tmp = t ? (*t * trf) : trf;
  glow_sPoint p1 = tmp * this->p1;
  glow_sPoint p2 = tmp * this->p2;

  p1.x = p1.x * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
  p1.y = p1.y * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
  p2.x = p2.x * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
  p2.y = p2.y * ctx->mw.zoom_factor_y - ctx->mw.offset_y;

  if (feq(p1.x, p2.x) && feq(p1.y, p2.y))
    return;

  ctx->export_jbean->line(p1.x, p1.y, p2.x, p2.y, draw_type, idx, pass,
      shape_cnt, node_cnt, fp);
  (*shape_cnt)++;
}

void GrowLine::flip(double x0, double y0, glow_eFlipDirection dir)
{
  switch (dir) {
  case glow_eFlipDirection_Horizontal:
    trf.store();
    set_scale(1, -1, x0, y0, glow_eScaleType_FixPoint);
    break;
  case glow_eFlipDirection_Vertical:
    trf.store();
    set_scale(-1, 1, x0, y0, glow_eScaleType_FixPoint);
    break;
  }
}

void GrowLine::convert(glow_eConvert version)
{
  switch (version) {
  case glow_eConvert_V34: {
    // Conversion of colors
    draw_type = GlowColor::convert(version, draw_type);
    original_border_drawtype
        = GlowColor::convert(version, original_border_drawtype);

    break;
  }
  }
}

void GrowLine::export_flow(GlowExportFlow* ef)
{
  ef->line(this);
}
