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

#include <stdlib.h>

#include <iostream>

#include "co_dcli.h"
#include "co_string.h"

#include "glow_growfolder.h"
#include "glow_grownode.h"
#include "glow_draw.h"

GrowFolder::GrowFolder(GrowCtx* glow_ctx, const char* name, double x, double y,
    double w, double h, glow_eDrawType border_d_type, int line_w,
    glow_eDrawType sel_color, glow_eDrawType unsel_color,
    glow_mDisplayLevel display_lev, int nodraw)
    : GrowWindow(glow_ctx, name, x, y, w, h, border_d_type, line_w, display_lev,
          nodraw),
      folders(2), text_size(1), text_drawtype(glow_eDrawType_TextHelveticaBold),
      text_color_drawtype(glow_eDrawType_Line), header_height(0.5),
      current_folder(0), color_selected(sel_color),
      color_unselected(unsel_color)
{
  for (int i = 0; i < MAX_FOLDERS; i++) {
    strcpy(folder_file_names[i], "");
    strcpy(folder_text[i], "");
    folder_scale[i] = 1;
    folder_v_scrollbar[i] = 0;
    folder_h_scrollbar[i] = 0;
    strcpy(folder_owner[i], "");
  }

  y_low_offs = header_height;

  if (!nodraw)
    ctx->set_dirty();
}

GrowFolder::~GrowFolder()
{
  ctx->set_dirty();
}

void GrowFolder::save(std::ofstream& fp, glow_eSaveMode mode)
{
  fp << int(glow_eSave_GrowFolder) << '\n';
  fp << int(glow_eSave_GrowFolder_folders) << FSPACE << folders << '\n';
  fp << int(glow_eSave_GrowFolder_text_size) << FSPACE << text_size << '\n';
  fp << int(glow_eSave_GrowFolder_text_drawtype) << FSPACE << text_drawtype
     << '\n';
  fp << int(glow_eSave_GrowFolder_text_color_drawtype) << FSPACE
     << text_color_drawtype << '\n';
  fp << int(glow_eSave_GrowFolder_header_height) << FSPACE << header_height
     << '\n';
  fp << int(glow_eSave_GrowFolder_color_selected) << FSPACE << color_selected
     << '\n';
  fp << int(glow_eSave_GrowFolder_color_unselected) << FSPACE
     << color_unselected << '\n';
  for (int i = 0; i < MAX_FOLDERS; i++) {
    fp << int(glow_eSave_GrowFolder_folder_file_names1) + i * 5 << FSPACE
       << folder_file_names[i] << '\n';
    fp << int(glow_eSave_GrowFolder_folder_text1) + i * 5 << FSPACE
       << folder_text[i] << '\n';
    fp << int(glow_eSave_GrowFolder_folder_scale1) + i * 5 << FSPACE
       << folder_scale[i] << '\n';
    fp << int(glow_eSave_GrowFolder_folder_v_scrollbar1) + i * 5 << FSPACE
       << folder_v_scrollbar[i] << '\n';
    fp << int(glow_eSave_GrowFolder_folder_h_scrollbar1) + i * 5 << FSPACE
       << folder_h_scrollbar[i] << '\n';
    fp << int(glow_eSave_GrowFolder_folder_owner1) + i << FSPACE
       << folder_owner[i] << '\n';
  }
  fp << int(glow_eSave_GrowFolder_window_part) << '\n';
  GrowWindow::save(fp, mode);
  fp << int(glow_eSave_End) << '\n';
}

void GrowFolder::open(std::ifstream& fp)
{
  int type = 0;
  int end_found = 0;
  char dummy[40];
  int tmp;

  for (;;) {
    if (!fp.good()) {
      fp.clear();
      fp.getline(dummy, sizeof(dummy));
      printf("** Read error GrowFolder: \"%d %s\"\n", type, dummy);
    }

    fp >> type;
    switch (type) {
    case glow_eSave_GrowFolder:
      break;
    case glow_eSave_GrowFolder_folders:
      fp >> folders;
      break;
    case glow_eSave_GrowFolder_text_size:
      fp >> text_size;
      break;
    case glow_eSave_GrowFolder_text_drawtype:
      fp >> tmp;
      text_drawtype = (glow_eDrawType)tmp;
      break;
    case glow_eSave_GrowFolder_text_color_drawtype:
      fp >> tmp;
      text_color_drawtype = (glow_eDrawType)tmp;
      break;
    case glow_eSave_GrowFolder_header_height:
      fp >> header_height;
      break;
    case glow_eSave_GrowFolder_color_selected:
      fp >> tmp;
      color_selected = (glow_eDrawType)tmp;
      break;
    case glow_eSave_GrowFolder_color_unselected:
      fp >> tmp;
      color_unselected = (glow_eDrawType)tmp;
      break;
    case glow_eSave_GrowFolder_folder_file_names1:
      fp.get();
      fp.getline(folder_file_names[0], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text1:
      fp.get();
      fp.getline(folder_text[0], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale1:
      fp >> folder_scale[0];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar1:
      fp >> folder_v_scrollbar[0];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar1:
      fp >> folder_h_scrollbar[0];
      break;
    case glow_eSave_GrowFolder_folder_file_names2:
      fp.get();
      fp.getline(folder_file_names[1], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text2:
      fp.get();
      fp.getline(folder_text[1], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale2:
      fp >> folder_scale[1];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar2:
      fp >> folder_v_scrollbar[1];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar2:
      fp >> folder_h_scrollbar[1];
      break;
    case glow_eSave_GrowFolder_folder_file_names3:
      fp.get();
      fp.getline(folder_file_names[2], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text3:
      fp.get();
      fp.getline(folder_text[2], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale3:
      fp >> folder_scale[2];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar3:
      fp >> folder_v_scrollbar[2];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar3:
      fp >> folder_h_scrollbar[2];
      break;
    case glow_eSave_GrowFolder_folder_file_names4:
      fp.get();
      fp.getline(folder_file_names[3], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text4:
      fp.get();
      fp.getline(folder_text[3], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale4:
      fp >> folder_scale[3];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar4:
      fp >> folder_v_scrollbar[3];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar4:
      fp >> folder_h_scrollbar[3];
      break;
    case glow_eSave_GrowFolder_folder_file_names5:
      fp.get();
      fp.getline(folder_file_names[4], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text5:
      fp.get();
      fp.getline(folder_text[4], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale5:
      fp >> folder_scale[4];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar5:
      fp >> folder_v_scrollbar[4];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar5:
      fp >> folder_h_scrollbar[4];
      break;
    case glow_eSave_GrowFolder_folder_file_names6:
      fp.get();
      fp.getline(folder_file_names[5], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text6:
      fp.get();
      fp.getline(folder_text[5], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale6:
      fp >> folder_scale[5];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar6:
      fp >> folder_v_scrollbar[5];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar6:
      fp >> folder_h_scrollbar[5];
      break;
    case glow_eSave_GrowFolder_folder_file_names7:
      fp.get();
      fp.getline(folder_file_names[6], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text7:
      fp.get();
      fp.getline(folder_text[6], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale7:
      fp >> folder_scale[6];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar7:
      fp >> folder_v_scrollbar[6];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar7:
      fp >> folder_h_scrollbar[6];
      break;
    case glow_eSave_GrowFolder_folder_file_names8:
      fp.get();
      fp.getline(folder_file_names[7], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text8:
      fp.get();
      fp.getline(folder_text[7], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale8:
      fp >> folder_scale[7];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar8:
      fp >> folder_v_scrollbar[7];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar8:
      fp >> folder_h_scrollbar[7];
      break;
    case glow_eSave_GrowFolder_folder_file_names9:
      fp.get();
      fp.getline(folder_file_names[8], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text9:
      fp.get();
      fp.getline(folder_text[8], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale9:
      fp >> folder_scale[8];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar9:
      fp >> folder_v_scrollbar[8];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar9:
      fp >> folder_h_scrollbar[8];
      break;
    case glow_eSave_GrowFolder_folder_file_names10:
      fp.get();
      fp.getline(folder_file_names[9], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text10:
      fp.get();
      fp.getline(folder_text[9], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale10:
      fp >> folder_scale[9];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar10:
      fp >> folder_v_scrollbar[9];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar10:
      fp >> folder_h_scrollbar[9];
      break;
    case glow_eSave_GrowFolder_folder_file_names11:
      fp.get();
      fp.getline(folder_file_names[10], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text11:
      fp.get();
      fp.getline(folder_text[10], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale11:
      fp >> folder_scale[10];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar11:
      fp >> folder_v_scrollbar[10];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar11:
      fp >> folder_h_scrollbar[10];
      break;
    case glow_eSave_GrowFolder_folder_file_names12:
      fp.get();
      fp.getline(folder_file_names[11], sizeof(folder_file_names[0]));
      break;
    case glow_eSave_GrowFolder_folder_text12:
      fp.get();
      fp.getline(folder_text[11], sizeof(folder_text[0]));
      break;
    case glow_eSave_GrowFolder_folder_scale12:
      fp >> folder_scale[11];
      break;
    case glow_eSave_GrowFolder_folder_v_scrollbar12:
      fp >> folder_v_scrollbar[11];
      break;
    case glow_eSave_GrowFolder_folder_h_scrollbar12:
      fp >> folder_h_scrollbar[11];
      break;
    case glow_eSave_GrowFolder_folder_owner1:
      fp.get();
      fp.getline(folder_owner[0], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner2:
      fp.get();
      fp.getline(folder_owner[1], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner3:
      fp.get();
      fp.getline(folder_owner[2], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner4:
      fp.get();
      fp.getline(folder_owner[3], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner5:
      fp.get();
      fp.getline(folder_owner[4], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner6:
      fp.get();
      fp.getline(folder_owner[5], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner7:
      fp.get();
      fp.getline(folder_owner[6], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner8:
      fp.get();
      fp.getline(folder_owner[7], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner9:
      fp.get();
      fp.getline(folder_owner[8], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner10:
      fp.get();
      fp.getline(folder_owner[9], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner11:
      fp.get();
      fp.getline(folder_owner[10], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_folder_owner12:
      fp.get();
      fp.getline(folder_owner[11], sizeof(folder_owner[0]));
      break;
    case glow_eSave_GrowFolder_window_part:
      GrowWindow::open(fp);
      break;
    case glow_eSave_End:
      end_found = 1;
      break;
    default:
      std::cout << "GrowFolder:open syntax error\n";
      fp.getline(dummy, sizeof(dummy));
    }
    if (end_found)
      break;
  }
  y_low_offs = header_height;
  strcpy(input_file_name, folder_file_names[0]);
  window_scale = folder_scale[0];
  vertical_scrollbar = folder_v_scrollbar[0];
  horizontal_scrollbar = folder_h_scrollbar[0];
  strcpy(owner, folder_owner[0]);
  GrowWindow::update_attributes();
}

void GrowFolder::draw(GlowWind* w, int ll_x, int ll_y, int ur_x, int ur_y)
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

void GrowFolder::draw(GlowWind* w, int* ll_x, int* ll_y, int* ur_x, int* ur_y)
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

void GrowFolder::set_highlight(int on)
{
  if (highlight != on) {
    highlight = on;
    ctx->set_dirty();
  }
}

void GrowFolder::draw(GlowWind* w, GlowTransform* t, int highlight, int hot,
    void* node, void* colornode)
{
  if (!(display_level & ctx->display_level))
    return;
  hot = (w == &ctx->navw) ? 0 : hot;
  int idx;
  glow_eDrawType drawtype;

  idx = int(w->zoom_factor_y / w->base_zoom_factor * line_width - 1);
  idx += hot;
  idx = MAX(0, idx);
  idx = MIN(idx, DRAW_TYPE_SIZE - 1);

  int text_idx
      = int(w->zoom_factor_y / w->base_zoom_factor * (text_size + 4) - 4);
  double tsize = w->zoom_factor_y / w->base_zoom_factor * (8 + 2 * text_size);
  text_idx = MIN(text_idx, DRAW_TYPE_SIZE - 1);

  Matrix tmp = t ? (*t * trf) : trf;
  glow_sPoint d1 = tmp * ll;
  glow_sPoint d2 = tmp * ur;
  d1.x = MIN(d1.x, d2.x);
  d2.x = MAX(d1.x, d2.x);
  d1.y = MIN(d1.y, d2.y);
  d2.y = MAX(d1.y, d2.y);

  int ll_x = int(d1.x * w->zoom_factor_x) - w->offset_x;
  int ur_x = int(d2.x * w->zoom_factor_x) - w->offset_x;
  int ur_y = int((d1.y + y_low_offs) * w->zoom_factor_y) - w->offset_y;
  int ll_y = int(d1.y * w->zoom_factor_y) - w->offset_y;

  drawtype = ctx->get_drawtype(draw_type, glow_eDrawType_LineHighlight,
      highlight, (GrowNode*)colornode, 0);
  glow_eDrawType drawtype_light
      = ctx->shift_drawtype(color_unselected, -2, (GrowNode*)colornode);
  glow_eDrawType drawtype_dark
      = ctx->shift_drawtype(color_unselected, 2, (GrowNode*)colornode);
  int x;
  int width = (ur_x - ll_x) / folders;
  int h = ur_y - ll_y;
  glow_sPointX p[4];
  for (int i = folders - 1; i >= -1; i--) {
    if (i == current_folder)
      // Draw this last
      continue;

    if (i == -1)
      i = current_folder;

    x = ll_x + i * width;
    if (i == 0)
      p[0].x = ll_x;
    else
      p[0].x = x - h / 4;
    p[0].y = ll_y + h;
    p[1].x = x + h / 4;
    p[1].y = ll_y;
    p[2].x = x + width - h / 4;
    p[2].y = ll_y;
    if (i == folders - 1)
      p[3].x = ur_x;
    else
      p[3].x = x + width + h / 4;
    p[3].y = ll_y + h;

    if (i == current_folder)
      ctx->gdraw->polyline(p, 4, color_selected, 1, 0);
    else {
      ctx->gdraw->polyline(p, 4, color_unselected, 1, 0);
      if (shadow) {
        ctx->gdraw->line(
            p[0].x + 1, p[0].y, p[1].x + 1, p[1].y, drawtype_light, 0, 0);
        if (i != 0) {
          glow_sPointX ps[4];

          ps[0].x = x + h / 4;
          ps[0].y = ll_y + h;
          ps[1].x = x;
          ps[1].y = ll_y + h / 2;
          ps[2].x = x + h / 8;
          ps[2].y = ll_y + h / 4;
          ps[3].x = x + h / 2;
          ps[3].y = ll_y + h;

          ctx->gdraw->polyline(ps, 4, drawtype_dark, 1, 0);
        }
      }
    }

    if (shadow) {
      if (i == current_folder) {
        drawtype_light = ctx->shift_drawtype(color_selected, -2,
            (GrowNode*)colornode);
        ctx->gdraw->line(
            p[0].x + 1, p[0].y, p[1].x + 1, p[1].y, drawtype_light, 0, 0);
      }
      ctx->gdraw->line(
          p[1].x, p[1].y + 1, p[2].x, p[2].y + 1, drawtype_light, 0, 0);
    }
    ctx->gdraw->polyline(p, 4, drawtype, 0, idx);

    if (text_idx >= 0) {
      ctx->gdraw->text(x + h / 2, ll_y + h - 2, folder_text[i],
          strlen(folder_text[i]), text_drawtype, text_color_drawtype, text_idx,
          highlight, glow_eFont_Helvetica, tsize, 0);
    }
    if (i == current_folder)
      break;
  }
  GrowWindow::draw(w, t, highlight, hot, node, colornode);
  ctx->gdraw->line(
      p[0].x + 1, p[0].y, p[3].x - 1, p[3].y, color_selected, idx, 0);
}

void GrowFolder::erase(GlowWind* w, GlowTransform* t, int hot, void* node)
{
  if (!(display_level & ctx->display_level))
    return;

  hot = (w == &ctx->navw) ? 0 : hot;
  int idx = int(w->zoom_factor_y / w->base_zoom_factor * line_width - 1);
  idx += hot;

  idx = MAX(0, idx);
  idx = MIN(idx, DRAW_TYPE_SIZE - 1);

  Matrix tmp = t ? (*t * trf) : trf;
  glow_sPoint p1 = tmp * ll;
  glow_sPoint p2 = tmp * ur;

  p1.x = p1.x * w->zoom_factor_x - w->offset_x;
  p1.y = p1.y * w->zoom_factor_y - w->offset_y;
  p2.x = p2.x * w->zoom_factor_x - w->offset_x;
  p2.y = p2.y * w->zoom_factor_y - w->offset_y;
  int ll_x = ROUND(MIN(p1.x, p2.x));
  int ur_x = ROUND(MAX(p1.x, p2.x));
  int ll_y = ROUND(MIN(p1.y, p2.y));
  int ur_y = ROUND(MAX(p1.y, p2.y));

  ctx->gdraw->rect(
      ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, glow_eDrawType_LineErase, 0, idx);
  ctx->gdraw->rect(
      ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, glow_eDrawType_LineErase, 1, 0);
}

void GrowFolder::export_javabean(GlowTransform* t, void* node,
    glow_eExportPass pass, int* shape_cnt, int node_cnt, int in_nc,
    std::ofstream& fp)
{
  Matrix tmp = t ? (*t * trf) : trf;
  glow_sPoint p1 = tmp * ll;
  glow_sPoint p2 = tmp * ur;

  p1.x = p1.x * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
  p1.y = p1.y * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
  p2.x = p2.x * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
  p2.y = p2.y * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
  double ll_x = MIN(p1.x, p2.x);
  double ur_x = MAX(p1.x, p2.x);
  double ll_y = MIN(p1.y, p2.y);
  double ur_y = MAX(p1.y, p2.y);

  ctx->export_jbean->folder(ll_x, ll_y, ur_x, ur_y, folders,
      (char*)folder_file_names, (char*)folder_text, (int*)folder_v_scrollbar,
      (int*)folder_h_scrollbar, owner, pass, shape_cnt, node_cnt, fp);
}

void GrowFolder::convert(glow_eConvert version)
{
  switch (version) {
  case glow_eConvert_V34: {
    // Conversion of colors
  }
  }
}

int GrowFolder::event_handler(glow_eEvent event, int x, int y, double fx, double fy)
{
  int sts;

  // Convert coordinates to local coordinates
  if (event == glow_eEvent_MB1Click) {
    if (x_left <= fx && fx <= x_right && y_low <= fy
        && fy <= y_low + header_height) {
      double w = (x_right - x_left) / folders;
      double x = x_left + w;
      for (int i = 0; i < folders; i++) {
        if (fx <= x) {
          // Change file
          strcpy(input_file_name, folder_file_names[i]);
          window_scale = folder_scale[i];
          vertical_scrollbar = folder_v_scrollbar[i];
          horizontal_scrollbar = folder_h_scrollbar[i];
          strcpy(owner, folder_owner[i]);
          current_folder = i;
          GrowWindow::update_attributes();
          ctx->set_dirty();
          break;
        }
        x += w;
      }
      return 1;
    }
  }
  sts = GrowWindow::event_handler(event, x, y, fx, fy);
  return sts;
}

int GrowFolder::update_attributes()
{
  y_low_offs = header_height;

  if (current_folder >= folders)
    current_folder = 0;

  if (!streq(folder_file_names[current_folder], "")) {
    strcpy(input_file_name, folder_file_names[current_folder]);
    window_scale = folder_scale[current_folder];
    vertical_scrollbar = folder_v_scrollbar[current_folder];
    horizontal_scrollbar = folder_h_scrollbar[current_folder];
    strcpy(owner, folder_owner[current_folder]);
    GrowWindow::update_attributes();
  }
  return 1;
}

void GrowFolder::set_textsize(int size)
{
  if (text_size != size) {
    text_size = size;
    ctx->set_dirty();
  }
}

void GrowFolder::set_textbold(int bold)
{
  if ((bold && draw_type == glow_eDrawType_TextHelveticaBold)
      || (!bold && draw_type == glow_eDrawType_TextHelvetica))
    return;

  if (bold)
    text_drawtype = glow_eDrawType_TextHelveticaBold;
  else
    text_drawtype = glow_eDrawType_TextHelvetica;
  ctx->set_dirty();
}

int GrowFolder::set_folder(int idx)
{
  if (idx >= folders || idx < 0)
    return 0;

  // Change file
  strcpy(input_file_name, folder_file_names[idx]);
  window_scale = folder_scale[idx];
  vertical_scrollbar = folder_v_scrollbar[idx];
  horizontal_scrollbar = folder_h_scrollbar[idx];
  strcpy(owner, folder_owner[idx]);
  current_folder = idx;
  GrowWindow::update_attributes();
  ctx->set_dirty();

  return 1;
}
