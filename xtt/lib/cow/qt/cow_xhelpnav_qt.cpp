/**
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2017 SSAB EMEA AB.
 *
 * This file is part of Proview.
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
 * along with Proview. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking Proview statically or dynamically with other modules is
 * making a combined work based on Proview. Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * Proview give you permission to, from the build function in the
 * Proview Configurator, combine Proview with modules generated by the
 * Proview PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every
 * copy of the combined work is accompanied by a complete copy of
 * the source code of Proview (the version used to produce the
 * combined work), being distributed under the terms of the GNU
 * General Public License plus this exception.
 **/

/* co_xhelpnav_qt.cpp -- helptext navigator */

#include "cow_qt_helpers.h"
#include "cow_xhelpnav_qt.h"

#include "flow_browwidget_qt.h"

void CoXHelpNavQt::pop()
{
}

CoXHelpNavQt::CoXHelpNavQt(void* xn_parent_ctx, QWidget* xn_parent_wid,
    char* xn_name, xhelp_eUtility xn_utility, QWidget** w, pwr_tStatus* status)
    : CoXHelpNav(xn_parent_ctx, xn_name, xn_utility, status)
{
  debug_print("creating a scrolledbrowwidgetqt\n");
  form_widget = scrolledbrowwidgetqt_new(
      CoXHelpNav::init_brow_base_cb, this, &brow_widget);

  showNow(brow_widget);

  displayed = 1;

  *w = form_widget;
  *status = 1;
}

CoXHelpNavQt::~CoXHelpNavQt()
{
  debug_print("CoXHelpNavQt::~CoXHelpNavQt\n");
  closing_down = 1;

  for (int i = 0; i < brow_cnt; i++) {
    if (i != 0) {
      brow_DeleteSecondaryCtx(brow_stack[i]->ctx);
    }
    brow_stack[i]->free_pixmaps();
    delete brow_stack[i];
  }
  delete brow;
  form_widget->close();
}

void CoXHelpNavQt::set_inputfocus()
{
  brow_widget->setFocus();
}