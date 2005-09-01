/* 
 * Proview   $Id: rt_fast.h,v 1.4 2005-09-01 14:57:48 claes Exp $
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

#ifndef rt_dsfast_h
#define rt_dsfast_h

using namespace std;

#include <string.h>
#include <vector>
extern "C" {
#include "pwr.h"
#include "pwr_baseclasses.h"
#include "rt_qcom.h"
}

/*! \file rt_dsfast.h
    \brief Contains the rt_dsfast class. */
/*! \addtogroup rt */
/*@{*/

#define FAST_CURVES 10
#define fast_cNoTrigg 0xffffffff

typedef enum {
  fast_mFunction_ManTrigg      	= 1 << 0,	//!< Allow manual trigg.
  fast_mFunction_LevelTrigg     = 1 << 1,	//!< Trigg on level.
  fast_mFunction_BeforeTrigg	= 1 << 2,	//!< Display points before trigg.
  fast_mFunction_AlwaysPrepared	= 1 << 3,	//!< Overwrite old curve as soon as the old is viewed.
  fast_mFunction_User		= 1 << 4	//!< Curve is handled by the user.
} fast_mFunction;

class fastobject {
 public:
  fastobject( pwr_sAttrRef *arp) : aref(*arp), p(0), trigg(0), time_buffer(0),
    old_level(0), stop_index(0), scan_div(0), scan_cnt(0)
    { memset( attributes, 0, sizeof(attributes));
    memset( buffers, 0, sizeof(buffers));}
  void open( double base_scantime);
  void close();
  void scan();

 private:
  pwr_sAttrRef	aref;
  pwr_sClass_DsFastCurve *p;
  pwr_tBoolean 	*trigg;
  void		*attributes[FAST_CURVES];
  void		*buffers[FAST_CURVES];
  void		*time_buffer;
  pwr_tDlid	p_dlid;
  pwr_tDlid	trigg_dlid;
  pwr_tDlid	attributes_dlid[FAST_CURVES];
  pwr_tDlid	buffers_dlid[FAST_CURVES];
  pwr_tDlid     time_buffer_dlid;
  pwr_tUInt32 	attributes_size[FAST_CURVES];
  pwr_tTime	prepare_time;
  pwr_tBoolean	old_prepare;
  pwr_tBoolean	old_trigg;
  int		current_index;
  int		new_cnt;
  pwr_tFloat32	old_level;
  int		stop_index;
  int		scan_div;
  int		scan_cnt;
  double	scan_base;
};

//! Handling of TrendCurve objects.
/*! ... 
*/

class rt_fast {
 public:
  rt_fast() : fast_cnt(0), scan_time(0.1)
    {}

  void init( qcom_sQid *qid);
  void open();
  void close();
  void scan();
  double scantime() { return scan_time;}

 private:
  vector<fastobject*>  objects;
  int		fast_cnt;
  double	scan_time;
};

/*@}*/
#endif




