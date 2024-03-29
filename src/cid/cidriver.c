/****************************************************************************
 *
 * cidriver.c
 *
 *   CID driver interface (body).
 *
 * Copyright (C) 1996-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include "cidriver.h"
#include "cidgload.h"
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftpsprop.h>

#include "ciderrs.h"

#include <freetype/internal/services/svpostnm.h>
#include <freetype/internal/services/svfntfmt.h>
#include <freetype/internal/services/svpsinfo.h>
#include <freetype/internal/services/svcid.h>
#include <freetype/internal/services/svprop.h>
#include <freetype/ftdriver.h>

#include <freetype/internal/psaux.h>


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  ciddriver


  /*
   * POSTSCRIPT NAME SERVICE
   *
   */

  static const char*
  cid_get_postscript_name( CID_Face  face )
  {
    const char*  result = face->cid.cid_font_name;


    if ( result && result[0] == '/' )
      result++;

    return result;
  }


  static const FT_TS_Service_PsFontNameRec  cid_service_ps_name =
  {
    (FT_TS_PsName_GetFunc)cid_get_postscript_name    /* get_ps_font_name */
  };


  /*
   * POSTSCRIPT INFO SERVICE
   *
   */

  static FT_TS_Error
  cid_ps_get_font_info( FT_TS_Face          face,
                        PS_FontInfoRec*  afont_info )
  {
    *afont_info = ((CID_Face)face)->cid.font_info;

    return FT_TS_Err_Ok;
  }

  static FT_TS_Error
  cid_ps_get_font_extra( FT_TS_Face          face,
                        PS_FontExtraRec*  afont_extra )
  {
    *afont_extra = ((CID_Face)face)->font_extra;

    return FT_TS_Err_Ok;
  }

  static const FT_TS_Service_PsInfoRec  cid_service_ps_info =
  {
    (PS_GetFontInfoFunc)   cid_ps_get_font_info,   /* ps_get_font_info    */
    (PS_GetFontExtraFunc)  cid_ps_get_font_extra,  /* ps_get_font_extra   */
    /* unsupported with CID fonts */
    (PS_HasGlyphNamesFunc) NULL,                   /* ps_has_glyph_names  */
    /* unsupported                */
    (PS_GetFontPrivateFunc)NULL,                   /* ps_get_font_private */
    /* not implemented            */
    (PS_GetFontValueFunc)  NULL                    /* ps_get_font_value   */
  };


  /*
   * CID INFO SERVICE
   *
   */
  static FT_TS_Error
  cid_get_ros( CID_Face      face,
               const char*  *registry,
               const char*  *ordering,
               FT_TS_Int       *supplement )
  {
    CID_FaceInfo  cid = &face->cid;


    if ( registry )
      *registry = cid->registry;

    if ( ordering )
      *ordering = cid->ordering;

    if ( supplement )
      *supplement = cid->supplement;

    return FT_TS_Err_Ok;
  }


  static FT_TS_Error
  cid_get_is_cid( CID_Face  face,
                  FT_TS_Bool  *is_cid )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;
    FT_TS_UNUSED( face );


    if ( is_cid )
      *is_cid = 1; /* cid driver is only used for CID keyed fonts */

    return error;
  }


  static FT_TS_Error
  cid_get_cid_from_glyph_index( CID_Face  face,
                                FT_TS_UInt   glyph_index,
                                FT_TS_UInt  *cid )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;
    FT_TS_UNUSED( face );


    if ( cid )
      *cid = glyph_index; /* identity mapping */

    return error;
  }


  static const FT_TS_Service_CIDRec  cid_service_cid_info =
  {
    (FT_TS_CID_GetRegistryOrderingSupplementFunc)
      cid_get_ros,                             /* get_ros                  */
    (FT_TS_CID_GetIsInternallyCIDKeyedFunc)
      cid_get_is_cid,                          /* get_is_cid               */
    (FT_TS_CID_GetCIDFromGlyphIndexFunc)
      cid_get_cid_from_glyph_index             /* get_cid_from_glyph_index */
  };


  /*
   * PROPERTY SERVICE
   *
   */

  FT_TS_DEFINE_SERVICE_PROPERTIESREC(
    cid_service_properties,

    (FT_TS_Properties_SetFunc)ps_property_set,      /* set_property */
    (FT_TS_Properties_GetFunc)ps_property_get )     /* get_property */


  /*
   * SERVICE LIST
   *
   */

  static const FT_TS_ServiceDescRec  cid_services[] =
  {
    { FT_TS_SERVICE_ID_FONT_FORMAT,          FT_TS_FONT_FORMAT_CID },
    { FT_TS_SERVICE_ID_POSTSCRIPT_FONT_NAME, &cid_service_ps_name },
    { FT_TS_SERVICE_ID_POSTSCRIPT_INFO,      &cid_service_ps_info },
    { FT_TS_SERVICE_ID_CID,                  &cid_service_cid_info },
    { FT_TS_SERVICE_ID_PROPERTIES,           &cid_service_properties },
    { NULL, NULL }
  };


  FT_TS_CALLBACK_DEF( FT_TS_Module_Interface )
  cid_get_interface( FT_TS_Module    module,
                     const char*  cid_interface )
  {
    FT_TS_UNUSED( module );

    return ft_service_list_lookup( cid_services, cid_interface );
  }



  FT_TS_CALLBACK_TABLE_DEF
  const FT_TS_Driver_ClassRec  t1cid_driver_class =
  {
    {
      FT_TS_MODULE_FONT_DRIVER       |
      FT_TS_MODULE_DRIVER_SCALABLE   |
      FT_TS_MODULE_DRIVER_HAS_HINTER,
      sizeof ( PS_DriverRec ),

      "t1cid",   /* module name           */
      0x10000L,  /* version 1.0 of driver */
      0x20000L,  /* requires FreeType 2.0 */

      NULL,    /* module-specific interface */

      cid_driver_init,          /* FT_TS_Module_Constructor  module_init   */
      cid_driver_done,          /* FT_TS_Module_Destructor   module_done   */
      cid_get_interface         /* FT_TS_Module_Requester    get_interface */
    },

    sizeof ( CID_FaceRec ),
    sizeof ( CID_SizeRec ),
    sizeof ( CID_GlyphSlotRec ),

    cid_face_init,              /* FT_TS_Face_InitFunc  init_face */
    cid_face_done,              /* FT_TS_Face_DoneFunc  done_face */
    cid_size_init,              /* FT_TS_Size_InitFunc  init_size */
    cid_size_done,              /* FT_TS_Size_DoneFunc  done_size */
    cid_slot_init,              /* FT_TS_Slot_InitFunc  init_slot */
    cid_slot_done,              /* FT_TS_Slot_DoneFunc  done_slot */

    cid_slot_load_glyph,        /* FT_TS_Slot_LoadFunc  load_glyph */

    NULL,                       /* FT_TS_Face_GetKerningFunc   get_kerning  */
    NULL,                       /* FT_TS_Face_AttachFunc       attach_file  */
    NULL,                       /* FT_TS_Face_GetAdvancesFunc  get_advances */

    cid_size_request,           /* FT_TS_Size_RequestFunc  request_size */
    NULL                        /* FT_TS_Size_SelectFunc   select_size  */
  };


/* END */
