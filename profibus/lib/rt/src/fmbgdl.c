/******************************************************************************
*                                                                             *
*                      SOFTING Industrial Automation GmbH                     *
*                          Richard-Reitzner-Allee 6                           *
*                                D-85540 Haar                                 *
*                        Phone: (++49)-(0)89-45656-0                          *
*                        Fax:   (++49)-(0)89-45656-399                        *
*                                                                             *
*            Copyright (C) SOFTING Industrial Automation GmbH 1995-2012       *
*                              All Rights Reserved                            *
*                                                                             *
*******************************************************************************

FILE_NAME               FMBGDL.C

PROJECT_NAME            PROFIBUS

MODULE                  FMBGDL

COMPONENT_LIBRARY       PAPI Lib
                        PAPI DLL

AUTHOR                  SOFTING

VERSION                 5.45.0.00.release

DATE                    October-2009

STATUS                  finished

FUNCTIONAL_MODULE_DESCRIPTION

This modul contains the Fieldbus Basic Management function which returns the
length of the request datas.



RELATED_DOCUMENTS
=============================================================================*/
#include "keywords.h"

INCLUDES

#if defined(WIN16) || defined(WIN32)
#include <windows.h>
#endif

#include "pb_type.h"
#include "pb_if.h"
#include "pb_err.h"
#include "pb_fmb.h"

GLOBAL_DEFINES

LOCAL_DEFINES

EXPORT_TYPEDEFS

LOCAL_TYPEDEFS

FUNCTION_DECLARATIONS

EXPORT_DATA

IMPORT_DATA

LOCAL_DATA

#if defined(WIN32) || defined(_WIN32) || defined(WIN16) || defined(_WIN16)
#pragma check_stack(off)
#endif

FUNCTION PUBLIC INT16 fmbgdl_get_data_len(
    IN INT16 result,         /* Service-Result */
    IN USIGN8 service,       /* Service */
    IN USIGN8 primitive,     /* Service-Primitive */
    IN USIGN8 FAR* data_ptr, /* pointer to data */
    OUT INT16* data_len_ptr  /* length of data */
    )

/*-----------------------------------------------------------------------------
FUNCTIONAL_DESCRIPTION

This function is used to return the request data length of FMB-SERVICES

possible return values:
- E_OK
- E_IF_INVALID_SERVICE

-----------------------------------------------------------------------------*/
{
  LOCAL_VARIABLES

  INT16 ret_val = E_OK;

  FUNCTION_BODY

  if (primitive == REQ)
  {
    switch (service)
    {
    case FMB_SET_CONFIGURATION:
    {
      T_FMB_SET_CONFIGURATION_REQ FAR* req_ptr =
          (T_FMB_SET_CONFIGURATION_REQ FAR*)data_ptr;
#ifdef WIN32
      req_ptr->sm7_active &= 0xFE;
#else
      req_ptr->sm7_active = PB_FALSE;
#endif
      *data_len_ptr = sizeof(T_FMB_SET_CONFIGURATION_REQ);
    }
    break;

    case FMB_SET_BUSPARAMETER:
      *data_len_ptr = sizeof(T_FMB_SET_BUSPARAMETER_REQ);
      break;

    case FMB_SET_VALUE:
    {
      T_FMB_SET_VALUE_REQ FAR* req = (T_FMB_SET_VALUE_REQ FAR*)data_ptr;
      *data_len_ptr = (sizeof(T_FMB_SET_VALUE_REQ) + req->length);
      break;
    }

    case FMB_READ_VALUE:
      *data_len_ptr = sizeof(T_FMB_READ_VALUE_REQ);
      break;

    case FMB_LSAP_STATUS:
      *data_len_ptr = sizeof(T_FMB_LSAP_STATUS_REQ);
      break;

    case FMB_VALIDATE_MASTER:
      *data_len_ptr = sizeof(T_FMB_VALIDATE_MASTER_REQ);
      break;

    case FMB_GET_LIVE_LIST:
    case FMB_READ_BUSPARAMETER:
    case FMB_EXIT:
    case FMB_RESET:
      *data_len_ptr = 0;
      break;

    default:
      *data_len_ptr = 0;
      ret_val = E_IF_INVALID_SERVICE;
      break;
    }
  }
  else
  {
    ret_val = E_IF_INVALID_PRIMITIVE;
  }

  return (ret_val);
}

#if defined(WIN32) || defined(_WIN32) || defined(WIN16) || defined(_WIN16)
#pragma check_stack
#endif
