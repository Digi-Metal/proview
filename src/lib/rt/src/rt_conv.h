/**
 * @file rt_conv.h
 *
 * Routines for converting attributes with different data types.
 *
 */

#ifndef rt_conv_h
#define rt_conv_h

typedef enum {
    conv_eIdx_invalid,
    conv_eIdx_copy,
    conv_eIdx_zero,

    conv_eIdx_boolToFloat32,
    conv_eIdx_boolToFloat64,
    conv_eIdx_boolToInt8,
    conv_eIdx_boolToInt16,
    conv_eIdx_boolToInt32,
 
    conv_eIdx_float32ToBoolean,
    conv_eIdx_float32ToFloat64,
    conv_eIdx_float32ToInt8,
    conv_eIdx_float32ToInt16,
    conv_eIdx_float32ToInt32,

    conv_eIdx_float64ToBoolean,
    conv_eIdx_float64ToFloat32,
    conv_eIdx_float64ToInt8,
    conv_eIdx_float64ToInt16,
    conv_eIdx_float64ToInt32,

    conv_eIdx_int8ToBoolean,
    conv_eIdx_int8ToFloat32,
    conv_eIdx_int8ToFloat64,
    conv_eIdx_int8ToInt16,
    conv_eIdx_int8ToInt32,
    conv_eIdx_int8ToUInt16,
    conv_eIdx_int8ToUInt32,

    conv_eIdx_int16ToBoolean,
    conv_eIdx_int16ToFloat32,
    conv_eIdx_int16ToFloat64,
    conv_eIdx_int16ToInt8,
    conv_eIdx_int16ToInt32,
    conv_eIdx_int16ToUInt8,
    conv_eIdx_int16ToUInt32,

    conv_eIdx_int32ToBoolean,
    conv_eIdx_int32ToFloat32,
    conv_eIdx_int32ToFloat64,
    conv_eIdx_int32ToInt8,
    conv_eIdx_int32ToInt16,
    conv_eIdx_int32ToUInt8,
    conv_eIdx_int32ToUInt16,

    conv_eIdx_uint8ToBoolean,
    conv_eIdx_uint8ToFloat32,
    conv_eIdx_uint8ToFloat64,
    conv_eIdx_uint8ToInt16,
    conv_eIdx_uint8ToInt32,
    conv_eIdx_uint8ToUInt16,
    conv_eIdx_uint8ToUInt32,

    conv_eIdx_uint16ToBoolean,
    conv_eIdx_uint16ToFloat32,
    conv_eIdx_uint16ToFloat64,
    conv_eIdx_uint16ToInt8,
    conv_eIdx_uint16ToInt32,
    conv_eIdx_uint16ToUInt8,
    conv_eIdx_uint16ToUInt32,

    conv_eIdx_uint32ToBoolean,
    conv_eIdx_uint32ToFloat32,
    conv_eIdx_uint32ToFloat64,
    conv_eIdx_uint32ToInt8,
    conv_eIdx_uint32ToInt16,
    conv_eIdx_uint32ToUInt8,
    conv_eIdx_uint32ToUInt16,

    conv_eIdx_stringToString,

    conv_eIdx_
} conv_eIdx;


typedef pwr_tBoolean (*convFunction) (
  int		tcount, /**< Number of target elements */
  int		tasize, /**< Size of one target element */
  char		*tp,    /**< Target address */
  int		*tsize, /**< Size of target buffer */
  int		scount, /**< Number of source elements */
  int		sasize, /**< Size of one source element */
  const char	*sp,    /**< Source address */
  pwr_mAdef	sadef   /**< Source flags, prevents conversion of private pointers */
  );


extern const convFunction conv_Fctn[];


conv_eIdx conv_GetIdx(pwr_eType src, pwr_eType trg);





#endif
