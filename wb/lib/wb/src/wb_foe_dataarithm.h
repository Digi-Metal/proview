/** 
 * Proview   $Id: wb_foe_dataarithm.h,v 1.2 2005-09-01 14:57:58 claes Exp $
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

#ifndef wb_foe_dataarithm_h
#define wb_foe_dataarithm_h

#ifndef pwr_h
#include "pwr.h"
#endif

pwr_tStatus dataarithm_convert (
  char *str,
  char *newstr,
  char *object, 
  int  bufsize,
  char *error_line,
  int  *error_line_size,
  int  *error_line_num,
  int  *outsize
);

#endif
