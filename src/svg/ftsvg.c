/****************************************************************************
 *
 * ftsvg.c
 *
 *   The FreeType SVG renderer interface (body).
 *
 * Copyright (C) 2022 by
 * David Turner, Robert Wilhelm, Werner Lemberg, and Moazin Khatti.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftserv.h>
#include <freetype/internal/services/svprop.h>
#include <freetype/otsvg.h>
#include <freetype/internal/svginterface.h>
#include <freetype/ftbbox.h>

#include "ftsvg.h"
#include "svgtypes.h"


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, usued to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  otsvg


#ifdef FT_TS_CONFIG_OPTION_SVG

  /* ft_svg_init */
  static FT_TS_Error
  ft_svg_init( SVG_Renderer  svg_module )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;


    svg_module->loaded    = FALSE;
    svg_module->hooks_set = FALSE;

    return error;
  }


  static void
  ft_svg_done( SVG_Renderer  svg_module )
  {
    if ( svg_module->loaded    == TRUE &&
         svg_module->hooks_set == TRUE )
      svg_module->hooks.free_svg( &svg_module->state );

    svg_module->loaded = FALSE;
  }


  static FT_TS_Error
  ft_svg_preset_slot( FT_TS_Module     module,
                      FT_TS_GlyphSlot  slot,
                      FT_TS_Bool       cache )
  {
    SVG_Renderer       svg_renderer = (SVG_Renderer)module;
    SVG_RendererHooks  hooks        = svg_renderer->hooks;


    if ( svg_renderer->hooks_set == FALSE )
    {
      FT_TS_TRACE1(( "Hooks are NOT set.  Can't render OT-SVG glyphs\n" ));
      return FT_TS_THROW( Missing_SVG_Hooks );
    }

    if ( svg_renderer->loaded == FALSE )
    {
      FT_TS_TRACE3(( "ft_svg_preset_slot: first presetting call,"
                  " calling init hook\n" ));
      hooks.init_svg( &svg_renderer->state );

      svg_renderer->loaded = TRUE;
    }

    return hooks.preset_slot( slot, cache, &svg_renderer->state );
  }


  static FT_TS_Error
  ft_svg_render( FT_TS_Renderer       renderer,
                 FT_TS_GlyphSlot      slot,
                 FT_TS_Render_Mode    mode,
                 const FT_TS_Vector*  origin )
  {
    SVG_Renderer  svg_renderer = (SVG_Renderer)renderer;

    FT_TS_Library  library = renderer->root.library;
    FT_TS_Memory   memory  = library->memory;
    FT_TS_Error    error;

    FT_TS_ULong  size_image_buffer;

    SVG_RendererHooks  hooks = svg_renderer->hooks;


    FT_TS_UNUSED( mode );
    FT_TS_UNUSED( origin );

    if ( mode != FT_TS_RENDER_MODE_NORMAL )
      return FT_TS_THROW( Bad_Argument );

    if ( svg_renderer->hooks_set == FALSE )
    {
      FT_TS_TRACE1(( "Hooks are NOT set.  Can't render OT-SVG glyphs\n" ));
      return FT_TS_THROW( Missing_SVG_Hooks );
    }

    if ( svg_renderer->loaded == FALSE )
    {
      FT_TS_TRACE3(( "ft_svg_render: first rendering, calling init hook\n" ));
      error = hooks.init_svg( &svg_renderer->state );

      svg_renderer->loaded = TRUE;
    }

    ft_svg_preset_slot( (FT_TS_Module)renderer, slot, TRUE );

    size_image_buffer = (FT_TS_ULong)slot->bitmap.pitch * slot->bitmap.rows;
    /* No `FT_TS_QALLOC` here since we need a clean, empty canvas */
    /* to start with.                                          */
    if ( FT_TS_ALLOC( slot->bitmap.buffer, size_image_buffer ) )
      return error;

    error = hooks.render_svg( slot, &svg_renderer->state );
    if ( error )
      FT_TS_FREE( slot->bitmap.buffer );
    else
      slot->internal->flags |= FT_TS_GLYPH_OWN_BITMAP;

    return error;
  }


  static const SVG_Interface  svg_interface =
  {
    (Preset_Bitmap_Func)ft_svg_preset_slot
  };


  static FT_TS_Error
  ft_svg_property_set( FT_TS_Module    module,
                       const char*  property_name,
                       const void*  value,
                       FT_TS_Bool      value_is_string )
  {
    FT_TS_Error      error    = FT_TS_Err_Ok;
    SVG_Renderer  renderer = (SVG_Renderer)module;


    if ( !ft_strcmp( property_name, "svg-hooks" ) )
    {
      SVG_RendererHooks*  hooks;


      if ( value_is_string == TRUE )
      {
        error = FT_TS_THROW( Invalid_Argument );
        goto Exit;
      }

      hooks = (SVG_RendererHooks*)value;

      if ( !hooks->init_svg    ||
           !hooks->free_svg    ||
           !hooks->render_svg  ||
           !hooks->preset_slot )
      {
        FT_TS_TRACE0(( "ft_svg_property_set:"
                    " SVG rendering hooks not set because\n" ));
        FT_TS_TRACE0(( "                    "
                    " at least one function pointer is NULL\n" ));

        error = FT_TS_THROW( Invalid_Argument );
        goto Exit;
      }

      renderer->hooks     = *hooks;
      renderer->hooks_set = TRUE;
    }
    else
      error = FT_TS_THROW( Missing_Property );

  Exit:
    return error;
  }


  static FT_TS_Error
  ft_svg_property_get( FT_TS_Module    module,
                       const char*  property_name,
                       const void*  value )
  {
    FT_TS_Error      error    = FT_TS_Err_Ok;
    SVG_Renderer  renderer = (SVG_Renderer)module;


    if ( !ft_strcmp( property_name, "svg-hooks" ) )
    {
      SVG_RendererHooks*  hooks = (SVG_RendererHooks*)value;


      *hooks = renderer->hooks;
    }
    else
      error = FT_TS_THROW( Missing_Property );

    return error;
  }


  FT_TS_DEFINE_SERVICE_PROPERTIESREC(
    ft_svg_service_properties,

    (FT_TS_Properties_SetFunc)ft_svg_property_set, /* set_property */
    (FT_TS_Properties_GetFunc)ft_svg_property_get  /* get_property */
  )


  FT_TS_DEFINE_SERVICEDESCREC1(
    ft_svg_services,
    FT_TS_SERVICE_ID_PROPERTIES, &ft_svg_service_properties )


  FT_TS_CALLBACK_DEF( FT_TS_Module_Interface )
  ft_svg_get_interface( FT_TS_Module    module,
                        const char*  ft_svg_interface )
  {
    FT_TS_Module_Interface  result;


    FT_TS_UNUSED( module );

    result = ft_service_list_lookup( ft_svg_services, ft_svg_interface );
    if ( result )
      return result;

    return 0;
  }


  static FT_TS_Error
  ft_svg_transform( FT_TS_Renderer       renderer,
                    FT_TS_GlyphSlot      slot,
                    const FT_TS_Matrix*  _matrix,
                    const FT_TS_Vector*  _delta )
  {
    FT_TS_SVG_Document  doc    = (FT_TS_SVG_Document)slot->other;
    FT_TS_Matrix*       matrix = (FT_TS_Matrix*)_matrix;
    FT_TS_Vector*       delta  = (FT_TS_Vector*)_delta;

    FT_TS_Matrix  tmp_matrix;
    FT_TS_Vector  tmp_delta;

    FT_TS_Matrix  a, b;
    FT_TS_Pos     x, y;


    FT_TS_UNUSED( renderer );

    if ( !matrix )
    {
      tmp_matrix.xx = 0x10000;
      tmp_matrix.xy = 0;
      tmp_matrix.yx = 0;
      tmp_matrix.yy = 0x10000;

      matrix = &tmp_matrix;
    }

    if ( !delta )
    {
      tmp_delta.x = 0;
      tmp_delta.y = 0;

      delta = &tmp_delta;
    }

    a = doc->transform;
    b = *matrix;
    FT_TS_Matrix_Multiply( &b, &a );


    x = ADD_LONG( ADD_LONG( FT_TS_MulFix( matrix->xx, doc->delta.x ),
                            FT_TS_MulFix( matrix->xy, doc->delta.y ) ),
                  delta->x );
    y = ADD_LONG( ADD_LONG( FT_TS_MulFix( matrix->yx, doc->delta.x ),
                            FT_TS_MulFix( matrix->yy, doc->delta.y ) ),
                  delta->y );

    doc->delta.x   = x;
    doc->delta.y   = y;
    doc->transform = a;

    return FT_TS_Err_Ok;
  }

#endif /* FT_TS_CONFIG_OPTION_SVG */


#ifdef FT_TS_CONFIG_OPTION_SVG
#define PUT_SVG_MODULE( a )  a
#define SVG_GLYPH_FORMAT     FT_TS_GLYPH_FORMAT_SVG
#else
#define PUT_SVG_MODULE( a )  NULL
#define SVG_GLYPH_FORMAT     FT_TS_GLYPH_FORMAT_NONE
#endif


  FT_TS_DEFINE_RENDERER(
    ft_svg_renderer_class,

      FT_TS_MODULE_RENDERER,
      sizeof ( SVG_RendererRec ),

      "ot-svg",
      0x10000L,
      0x20000L,

      (const void*)PUT_SVG_MODULE( &svg_interface ), /* module specific interface */

      (FT_TS_Module_Constructor)PUT_SVG_MODULE( ft_svg_init ), /* module_init   */
      (FT_TS_Module_Destructor)PUT_SVG_MODULE( ft_svg_done ),  /* module_done   */
      PUT_SVG_MODULE( ft_svg_get_interface ),               /* get_interface */

      SVG_GLYPH_FORMAT,

      (FT_TS_Renderer_RenderFunc)   PUT_SVG_MODULE( ft_svg_render ),    /* render_glyph    */
      (FT_TS_Renderer_TransformFunc)PUT_SVG_MODULE( ft_svg_transform ), /* transform_glyph */
      NULL,                                                          /* get_glyph_cbox  */
      NULL,                                                          /* set_mode        */
      NULL                                                           /* raster_class    */
  )


/* END */