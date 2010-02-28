/* 
 * Proview   $Id: rt_plc_macro_io.h,v 1.9 2008-09-05 08:57:44 claes Exp $
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

/*		 PREPROCESSOR RUTINER					    */

/*_*
  STODO								
  store digital output
  @aref stodo StoDo
*/
#define stodo_exec(obj,in)						\
  obj->ActualValue = in;

/*_*
  STODV								
  store digital value				
  @aref stodv StoDv
*/
#define stodv_exec(obj,in)						\
  obj->ActualValue = in;

/*_*
  STODP								
  Store into digital parameter			
  @aref stodp StoDp
*/
#define stodp_exec(ut,in)						\
  ut = in;

/*_*
  STOAO								
  store into analog output			
  @aref stoao StoAo
*/
#define stoao_exec(obj,in)						\
  obj->ActualValue = in;

/*_*
  STOAV								
  store into analog value				
  @aref stoav StoAv
*/
#define stoav_exec(obj,in)						\
  obj->ActualValue = in;

/*_*
  STOAP								
  Store into analog parameter			
  @aref stoap StoAp
*/
#define stoap_exec(ut,in)						\
  ut = in;

/*_*
  CSTOAO								
  store conditionally into analog output		
  @aref cstoao CStoAo
*/
#define cstoao_exec(obj,in,cond)					\
  if ( cond ) obj->ActualValue = in;

/*_*
  CSTOAV								
  store conditionally into analog value		
  @aref cstoav CStoAv
*/
#define cstoav_exec(obj,in,cond)					\
  if ( cond ) obj->ActualValue = in;

/*_*
  CSTOAP								
  Store conditionally into analog parameter	
  @aref cstoap CStoAp
*/
#define cstoap_exec(ut,in,cond)						\
  if ( cond ) ut = in;

/*_*
  SETDO								
  Set digital output if true			
  @aref setdo SetDo
*/
#define setdo_exec(obj,in)						\
  if ( in ) obj->ActualValue = true;

/*_*
  SETDV								
  Set digital value if true			
  @aref setdv SetDv
*/
#define setdv_exec(obj,in)						\
  if ( in ) obj->ActualValue = true;

/*_*
  SETDP								
  Set digital parameter if true			
  @aref setdp SetDp
*/
#define setdp_exec(ut,in)						\
  if ( in ) ut = true;

/*_*
  RESDO								
  Reset digital output if true			
  @aref resdo ResDo
*/
#define resdo_exec(obj,in)						\
  if ( in ) obj->ActualValue = false;

/*_*
  RESDV								
  Reset digital value if true			
  @aref resdv ResDv
*/
#define resdv_exec(obj,in)						\
  if ( in ) obj->ActualValue = false;

/*_*
  RESDP								
  Reset digital parameter if true			
  @aref resdp ResDp
*/
#define resdp_exec(ut,in)						\
  if ( in ) ut = false;

/*_*
  StoIp                                                           
  Store integer parameter                    
  @aref stoip StoIp
*/
#define StoIp_exec(ut,in)                                               \
  ut = in;

/*_*
  CStoIp                                                          
  Store conditionally integer parameter      
  @aref cstoip CStoIp
*/
#define CStoIp_exec(ut,in,cond)                                         \
  if ( cond ) ut = in;

/*_*
  StoAtoIp                                                           
  Store analog value into integer parameter                    
  @aref stoatoip StoAtoIp
*/
#define StoAtoIp_exec(ut,in)                                               \
  ut = in > 0 ? in + 0.5 : in - 0.5;

/*_*
  CStoAtoIp                                                          
  Store conditionally analog value into integer parameter      
  @aref cstoatoip CStoAtoIp
*/
#define CStoAtoIp_exec(ut,in,cond)                                         \
  if ( cond ) ut = in > 0 ? in + 0.5 : in - 0.5;

/*_*
  GetIpToA                                                           
  Get Integer parameter as an analog value             
  @aref getiptoa GetIpToA
*/
#define GetIpToA_exec(object,in)                                           \
  object->ActVal = in;

/*_*
  GetIp                                                           
  Get Integer parameter             
  @aref getip GetIp
*/
#define GetIp_exec(object,in)                                           \
  object->ActVal = in;

/*_*
  STODI								
  store digital input (Simulate)			
  @aref stodi StoDi
*/
#define stodi_exec(obj,in)						\
  obj->ActualValue = in;

/*_*
  SETDI								
  Set digital input if true (Simulate)		
  @aref setdi SetDi
*/
#define setdi_exec(obj,in)						\
  if ( in ) obj->ActualValue = true;

/*_*
  RESDI								
  Reset digital input if true (Simulate)		
  @aref resdi ResDi
*/
#define resdi_exec(obj,in)						\
  if ( in ) obj->ActualValue = false;

/*_*
  TOGGLEDI								
  Toggle digital input (Simulate)			
  @aref toggledi ToggleDi
*/
#define toggledi_exec(obj,in)						\
  if ( in) obj->ActualValue = !obj->ActualValue;

/*_*
  STOAI								
  store analog input (Simulate)			
  @aref stoai StoAi
*/
#define stoai_exec(obj,in)						\
  obj->ActualValue = in;

/*_*
  CSTOAI								
  store conditionally into analog input (Simulate) 
  @aref cstoai CStoAi
*/
#define cstoai_exec(obj,in,cond)					\
  if ( cond ) obj->ActualValue = in;

/*_*
  STOPI								
  store into co (Simulate) 
  @aref stopi StoPi
*/
#define stopi_exec( rawvalue, absvalue ,in)				\
  rawvalue->RawValue = in;						\
  absvalue->RawValue = in;

/*_*
  StoIo					
  Store integer output	
  @aref stoio StoIo
*/
#define stoio_exec(obj,in)						\
  obj->ActualValue = in;

/*_*
  CStoIo							
  store conditionally into integer output 
  @aref cstoio CStoIo
*/
#define cstoio_exec(obj,in,cond)					\
  if ( cond ) obj->ActualValue = in;

/*_*
  StoIv
  Store integer value	
  @aref stoiv StoIv
*/
#define stoiv_exec(obj,in)						\
  obj->ActualValue = in;

/*_*
  CStoIv
  store conditionally into integer value 
  @aref cstoiv CStoIv
*/
#define cstoiv_exec(obj,in,cond)					\
  if ( cond ) obj->ActualValue = in;

/*_*
  StoIi								
  store integer input (Simulate)			
  @aref stoii StoIi
*/
#define stoii_exec(obj,in)						\
  obj->ActualValue = in;

/*_*
  CStoIi								
  store conditionally into integer input (Simulate) 
  @aref cstoii CStoIi
*/
#define cstoii_exec(obj,in,cond)					\
  if ( cond ) obj->ActualValue = in;

/*_*
  AtoI
  @aref atoi AtoI
*/
#define AtoI_exec(obj,in) \
  obj->ActVal = in > 0 ? in + 0.5 : in - 0.5;

/*_*
  ItoA
  @aref itoa ItoA
*/
#define ItoA_exec(obj,in) \
  obj->ActVal = in;

/*_*
  StoDattr
  @aref stodattr StoDattr
*/
#define StoDattr_exec(attr,in) \
  attr = in;

/*_*
  SetDattr
  @aref setdattr SetDattr
*/
#define SetDattr_exec(attr,in) \
  if ( in) attr = true;

/*_*
  ResDattr
  @aref resdattr ResDattr
*/
#define ResDattr_exec(attr,in) \
  if ( in) attr = false;

/*_*
  StoIattr
  @aref stoiattr StoIattr
*/
#define StoIattr_exec(attr,in) \
  attr = in;

/*_*
  CStoIattr
  @aref cstoiattr CStoIattr
*/
#define CStoIattr_exec(attr,in,cond) \
  if ( cond) attr = in;

/*_*
  StoAattr
  @aref stoaattr StoAattr
*/
#define StoAattr_exec(attr,in) \
  attr = in;

/*_*
  CStoAattr
  @aref cstoaattr CStoAattr
*/
#define CStoAattr_exec(attr,in,cond) \
  if ( cond) attr = in;

/*_*
  StoSattr
  @aref stosattr StoSattr
*/
#define StoSattr_exec(attr,in,size) \
  strncpy( attr, in, size); \
  attr[size-1] = 0;

/*_*
  CStoSattr
  @aref cstosattr CStoSattr
*/
#define CStoSattr_exec(attr,in,cond,size) \
  if ( cond) { \
    strncpy( attr, in, size); \
    attr[size-1] = 0; \
  }

/*_*
  CStoExtBoolean
  @aref cstoextboolean CStoExtBoolean
*/
#define CStoExtBoolean_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tBoolean)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtFloat32
  @aref cstoextfloat32 CStoExtFloat32
*/
#define CStoExtFloat32_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tFloat32)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtFloat64
  @aref cstoextfloat64 CStoExtFloat64
*/
#define CStoExtFloat64_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tFloat64)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtInt8
  @aref cstoextint8 CStoExtInt8
*/
#define CStoExtInt8_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tInt8)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtInt16
  @aref cstoextint16 CStoExtInt16
*/
#define CStoExtInt16_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tInt16)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtInt32
  @aref cstoextint32 CStoExtInt32
*/
#define CStoExtInt32_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tInt32)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtInt64
  @aref cstoextint64 CStoExtInt64
*/
#define CStoExtInt64_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tInt64)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtInt8
  @aref cstoextuint8 CStoExtUInt8
*/
#define CStoExtUInt8_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tUInt8)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtUInt16
  @aref cstoextuint16 CStoExtUInt16
*/
#define CStoExtUInt16_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tUInt16)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtUInt32
  @aref cstoextuint32 CStoExtUInt32
*/
#define CStoExtUInt32_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tUInt32)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtUInt64
  @aref cstoextuint64 CStoExtUInt64
*/
#define CStoExtUInt64_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tUInt64)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtString
  @aref cstoextstring CStoExtString
*/
#define CStoExtString_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tString80)); \
  } \
  obj->OldCond = cond;

/*_*
  CStoExtTime
  @aref cstoexttime CStoExtTime
*/
#define CStoExtTime_exec(obj, name, in, cond) \
  if ( cond && !obj->OldCond) { \
    obj->LastStatus = gdh_SetObjectInfo( name, (void *)&in, sizeof(pwr_tTime)); \
  } \
  obj->OldCond = cond;

/*_*
  GetExtFloat32
  @aref getextfloat32 GetExtFloat32
*/
#define GetExtFloat32_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tFloat32)); \
}

#define GetExtFloat32_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  GetExtFloat64
  @aref getextfloat64 GetExtFloat64
*/
#define GetExtFloat64_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tFloat64)); \
}

#define GetExtFloat64_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  GetExtInt64
  @aref getextint64 GetExtInt64
*/
#define GetExtInt64_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tInt64)); \
}

#define GetExtInt64_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  GetExtUInt64
  @aref getextuint64 GetExtUInt64
*/
#define GetExtUInt64_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tUInt64)); \
}

#define GetExtUInt64_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;


/*_*
  GetExtInt32
  @aref getextint32 GetExtInt32
*/
#define GetExtInt32_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tInt32)); \
}

#define GetExtInt32_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  GetExtUInt32
  @aref getextuint32 GetExtUInt32
*/
#define GetExtUInt32_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tUInt32)); \
}

#define GetExtUInt32_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  GetExtInt16
  @aref getextint16 GetExtInt16
*/
#define GetExtInt16_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tInt16)); \
}

#define GetExtInt16_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  GetExtUInt16
  @aref getextuint16 GetExtUInt16
*/
#define GetExtUInt16_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tUInt16)); \
}

#define GetExtUInt16_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  GetExtInt8
  @aref getextint8 GetExtInt8
*/
#define GetExtInt8_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tInt8)); \
}

#define GetExtInt8_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  GetExtUInt8
  @aref getextuint8 GetExtUInt8
*/
#define GetExtUInt8_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tUInt8)); \
}

#define GetExtUInt8_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  GetExtBoolean
  @aref getextboolean GetExtBoolean
*/
#define GetExtBoolean_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tBoolean)); \
}

#define GetExtBoolean_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  GetExtString
  @aref getextstring GetExtString
*/
#define GetExtString_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tString80)); \
}

#define GetExtString_exec(obj) \
  if ( obj->ExtP) \
    strncpy( obj->ActVal, obj->ExtP, sizeof(pwr_tString80));

/*_*
  GetExtTime
  @aref getexttime GetExtTime
*/
#define GetExtTime_init(obj, name) \
{ \
  gdh_RefObjectInfo( name, &obj->ExtP, 0, sizeof(pwr_tTime)); \
}

#define GetExtTime_exec(obj) \
  if ( obj->ExtP) \
    obj->ActVal = *obj->ExtP;

/*_*
  Float64toA
  @aref float64toa Float64toA
*/
#define Float64toA_exec(obj,in) \
  obj->ActVal = in;

/*_*
  AtoFloat64
  @aref atofloat64 AtoFloat64
*/
#define AtoFloat64_exec(obj,in) \
  obj->ActVal = in;

/*_*
  ItoUInt32
  @aref itouint32 ItoUInt32
*/
#define ItoUInt32_exec(obj,in) \
  obj->ActVal = in;

/*_*
  UInt32toI
  @aref uint32toi UInt32toI
*/
#define UInt32toI_exec(obj,in) \
  obj->ActVal = in;

/*_*
  Int64toI
  @aref int64toi Int64toI
*/
#if defined OS_LINUX || defined OS_MACOS
#define Int64toI_exec(obj,in) \
  obj->ActVal = in;
#else
#define Int64toI_exec(obj,in) \
  obj->ActVal = (0x80000000 & in.high) | (0xEFFFFFFF & in.low);
#endif

/*_*
  ItoInt64t
  @aref itoint64 ItoInt64
*/
#if defined OS_LINUX || defined OS_MACOS
#define ItoInt64_exec(obj,in) \
  obj->ActVal = in;
#else
#define ItoInt64_exec(obj,in) \
  obj->ActVal.high = in & 0x80000000;
  obj->ActVal.low = in & 0xEFFFFFFF;
#endif

/*_*
  UInt64toI
  @aref uint64toi UInt64toI
*/
#if defined OS_LINUX || defined OS_MACOS
#define UInt64toI_exec(obj,in) \
  obj->ActVal = in;
#else
#define UInt64toI_exec(obj,in) \
  obj->ActVal = in.low;
#endif

/*_*
  ItoUInt64
  @aref itouint64toi ItoUInt64
*/
#if defined OS_LINUX || defined OS_MACOS
#define ItoUInt64_exec(obj,in) \
  obj->ActVal = in;
#else
#define ItoUInt64_exec(obj,in) \
  obj->ActVal.low = in;
  obj->ActVal.high = 0;
#endif
