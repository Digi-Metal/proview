/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2016 SSAB EMEA AB.
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

#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include "pwr.h"
#include "co_time.h"
#include "rt_gdh.h"
#include "pwr_baseclasses.h"
#include "sev_valuecache.h"

const int sev_valuecache::m_size = VALUECACHE_SIZE;

int sev_valuecache::idx( int index)
{
  if ( !m_length)
    return 0;
  if ( index >= m_length)
    return 0;
  int i = m_last - index;
  if ( i < 0)
    i += m_size;
  return i;  
}

sev_sCacheValue& sev_valuecache::operator[]( const int index)
{
  return m_val[idx(index)];
}

void sev_valuecache::add( double val, pwr_tTime *t)
{
  double time;
  pwr_tDeltaTime dt;

  time_Adiff_NE( &dt, t, &m_start_time);
  time = time_DToFloat64( 0, &dt);

  // Store optimized write index before adding
  m_last_opt_write = get_optimal_write();

  bool update_k = m_length < m_size;
  if ( !m_length) {
    m_val[0].val = val;
    m_val[0].time = time;
    m_length++;
  }
  else {
    if ( ++m_last >= m_size)
      m_last -= m_size;
    m_val[m_last].val = val;
    m_val[m_last].time = time;
    m_length++;
    if ( m_last == m_first) {
      m_first++;
      if ( m_first >= m_size)
	m_first -= m_size;	 
      m_length--;
    }
  }
  if ( !m_inited) {
    write( 0);
    m_inited = true;
    return;
  }

  if ( update_k) {
    calculate_k();
    // Update epsilon for all data
    calculate_epsilon();
  }
  else 
    calculate_epsilon(0);
}

void sev_valuecache::evaluate() 
{
  int value_added = 1;

  while( 1) {
    if ( !check_deadband()) {
      // Store optimal value
      write( m_last_opt_write  + value_added);
    }
    else
      break;

    calculate_k();
    calculate_epsilon();
    m_last_opt_write = get_optimal_write();
    value_added = 0;
  }
}

void sev_valuecache::calculate_k()
{
  double xysum = 0;
  double x2sum = 0;  

  for ( int i = 0; i < length(); i++) {
    int ii = idx(i);
    xysum += (m_val[ii].val - m_wval.val) * (m_val[ii].time - m_wval.time); 
    x2sum += (m_val[ii].val - m_wval.val) * (m_val[ii].val - m_wval.val); 
  }
  if ( x2sum < DBL_EPSILON) {
    m_k = 1E32;
    m_m = m_wval.time;
    m_deadband = m_deadband_time;
  }
  else {
    m_k = x2sum/xysum;
    m_m = m_wval.val - m_wval.time * m_k;
    m_deadband = m_deadband_value + 
      fabs(atan(m_k)) / (M_PI/2) * ( m_deadband_time - m_deadband_value); 
  }
}

void sev_valuecache::write( int index) 
{
  int ii = idx(index);
  double wval, wtime;

  if ( m_type == sev_eCvType_Mean) {
    if ( fabs(m_last_k) < 1) {
      m_wval.val = m_wval.val + m_last_k * (m_val[ii].time - m_wval.time);
      m_wval.time = m_val[ii].time;
    }
    else {
      m_wval.time = m_wval.time + (m_val[ii].val - m_wval.val) / m_last_k;
      m_wval.val = m_val[ii].val;
    }
    wval = m_wval.val;
    wtime = m_wval.time;    
  }
  else {
    wval = m_val[ii].val;
    wtime = m_val[ii].time;
    m_wval = m_val[ii];
  }

  if ( index == 0) {
    m_last = m_first = 0;
    m_length = 0;
  }
  else {
    m_first = ii + 1;
    if ( m_first >= m_size)
      m_first -= m_size;
    m_length = m_last - m_first + 1;
    if ( m_length < 0)
      m_length += m_size;
  }
  if ( m_write_cb) {
    pwr_tTime time;
    time_Aadd( &time, &m_start_time, time_Float64ToD( 0, wtime));
    (m_write_cb)( m_userdata, m_useridx, wval, &time);
  }
}

// Calculate epsilon for all
void sev_valuecache::calculate_epsilon()
{
  if ( m_length == 1) {
    m_val[m_first].epsilon = 0;
    return;
  }

  for ( int i = 0; i < m_length; i++)
    calculate_epsilon(i);
}
 
// Calculate epsilon for one index
void sev_valuecache::calculate_epsilon( int index)
{
  int ii = idx(index);
  if ( m_k >= 1E32) {
    m_val[ii].epsilon = fabs( m_val[ii].time - m_wval.time);
  }
  else {
    m_val[ii].epsilon = fabs(m_val[ii].val - m_k * m_val[ii].time - m_m) / sqrt( 1 + m_k * m_k);
  }
}

// Check deadband for one index
// Returns true if all values inside deadband.
bool sev_valuecache::check_deadband( int index)
{
  if ( m_val[idx(index)].epsilon > m_deadband)
    return false;
  return true;
}

// Check deadband for all values
// Returns true if all values inside deadband.
bool sev_valuecache::check_deadband()
{
  for ( int i = 0; i < m_length; i++) {
    int ii = idx(i);
    if ( m_val[ii].epsilon > m_deadband)
      return false;
  }
  return true;
}

int sev_valuecache::get_optimal_write()
{
  if ( m_type == sev_eCvType_Mean) {
    m_last_k = m_k;
    return 0;
  }
  
  double min_weight = 10E32;
  int ii;
  double dist;
  double weight;
  int min_idx = 0;

  for ( int i = 0; i < m_length; i++) {
    if ( m_length == m_size && i == m_length - 1)
      continue;

    ii = idx(i);
    dist = sqrt( (m_val[ii].val - m_wval.val) * (m_val[ii].val - m_wval.val) + 
		 (m_val[ii].time - m_wval.time) * (m_val[ii].time - m_wval.time));
    weight = m_val[ii].epsilon / dist;
    if ( weight < min_weight) {
      min_weight = weight;
      min_idx = i;
    }
  }
  return min_idx;
}
