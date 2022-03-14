/****************************************************************************
 *
 * ftsynth.c
 *
 *   FreeType synthesizing code for emboldening and slanting (body).
 *
 * Copyright (C) 2000-2021 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include <freetype/ftsynth.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/ftoutln.h>
#include <freetype/ftbitmap.h>


  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  synth


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****   EXPERIMENTAL OBLIQUING SUPPORT                                ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

  /* documentation is in ftsynth.h */

/**
 TSIT {{{{{{{{{{
 */

  FT_EXPORT_DEF( int )
  FT_GlyphSlot_Get_Matrix_Degree(FT_Matrix* pMatrix)
  {
    if (!pMatrix)
    {
        return 0;
    }

    if (  pMatrix->xx ==  0
       && pMatrix->xy ==  0x10000
       && pMatrix->yx == -0x10000
       && pMatrix->yy ==  0)
    {
       return 90;
    }

    if (pMatrix->xx ==  0
     && pMatrix->xy == -0x10000
     && pMatrix->yx ==  0x10000
     && pMatrix->yy ==  0 )
    {
       return -90;
    }
    return 0;
  }

  FT_EXPORT_DEF( void )
  FT_GlyphSlot_Oblique( FT_GlyphSlot  slot )
  {
    FT_GlyphSlot_Oblique_Direction(slot, FT_FONT_ITALIC_VALUE, FT_POSTURE_TO_RIGHT);
  }

  FT_EXPORT_DEF( void )
  FT_GlyphSlot_Oblique_Direction( FT_GlyphSlot  slot, float oblique, int flags )
  {
    FT_Matrix    transform;
    FT_Outline*  outline;
    FT_Library   library;
    FT_Face      face;
    FT_Pos       xstr, ystr;
    int          to_bottom;
    int          degree;
    int          transvalue;

    if ( !slot )
      return;

    face    = slot->face;
    if ( slot->format != FT_GLYPH_FORMAT_OUTLINE )
    {
        return;
    }

    /* we don't touch the advance width */

    /* For italic, simply apply a shear transform, with an angle */
    /* of about 12 degrees.                                      */
    /* if text both have directory and italic,
       text directory is solid attibute.
       italic is temporary attribute.

       so, I must change transform for directory.
       0x0366AL/0x10000L=0.21
     */
    degree     = FT_GlyphSlot_Get_Matrix_Degree( &(face->internal->transform_matrix));
    to_bottom  = FT_POSTURE_TO_BOTTOM_CHECK(flags);
    /* if is oblique=0.21256, transvalue=0x0366AL*/
    transvalue = (int)(0x10000L*oblique);
    if (   degree ==   0 &&  to_bottom
        || degree ==  90 && !to_bottom
        || degree == -90 && !to_bottom )
    {
        transform.xx =  0x10000L;
        transform.yx = -transvalue;
        transform.xy =  0x00000L;
        transform.yy =  0x10000L;
    }
    else
    {
        transform.xx =  0x10000L;
        transform.yx =  0x00000L;
        transform.xy =  transvalue;
        transform.yy =  0x10000L;
    }

    outline = &slot->outline;
    FT_Outline_Transform( outline, &transform );
  }

  FT_EXPORT_DEF( void )
  FT_GlyphSlot_Transform( FT_GlyphSlot  slot )
  {
    FT_Matrix    transform;
    FT_Outline*  outline;
    FT_Library   library;
    FT_Face      face;
    FT_Pos       xstr, ystr;
    int degree = 0;

    if ( !slot )
      return;

    face    = slot->face;

    if ( slot->format != FT_GLYPH_FORMAT_OUTLINE )
    {
        return;
    }

    outline = &slot->outline;
    FT_Outline_Transform( outline, &transform );
  }

  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****   EXPERIMENTAL EMBOLDENING SUPPORT                              ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/


  /* documentation is in ftsynth.h */

  FT_EXPORT_DEF( void )
  FT_GlyphSlot_Embolden( FT_GlyphSlot  slot )
  {
    FT_GlyphSlot_Weight(slot, FT_WEIGHT_BOLD, FT_WEIGHT_BOLD, 0);
  }

  FT_EXPORT_DEF( void )
  FT_GlyphSlot_Weight( FT_GlyphSlot  slot, float weight_x, float weight_y, int flags )
  {
    FT_Library  library;
    FT_Face     face;
    FT_Error    error;
    FT_Pos      xstr, ystr;
    int            changex, changey;

    if ( !slot )
      return;

    library = slot->library;
    face    = slot->face;

    if ( slot->format != FT_GLYPH_FORMAT_OUTLINE &&
         slot->format != FT_GLYPH_FORMAT_BITMAP  )
      return;

    /* some reasonable strength */
    xstr = FT_MulFix( face->units_per_EM,
                      face->size->metrics.y_scale ) / 24;
    ystr = xstr;

    /* different weight cause different thick.
     * weight plain(1) means no process, so we minus it.
     */
    weight_x = weight_x - FT_WEIGHT_PLAIN;
    xstr     = (int)(xstr * weight_x);
    weight_y = weight_y - FT_WEIGHT_PLAIN;
    ystr     = (int)(ystr * weight_y);

    if ( slot->format == FT_GLYPH_FORMAT_OUTLINE )
    {
      FT_Outline_WeightXY( &slot->outline, xstr, ystr );
    }
    else /* slot->format == FT_GLYPH_FORMAT_BITMAP */
    {
      /* round to full pixels */
      xstr &= ~63;
      if ( xstr == 0 )
        xstr = 1 << 6;
	  xstr     = (int)(xstr * weight_x);

      ystr &= ~63;
	  ystr     = (int)(ystr * weight_y);

      /*
       * XXX: overflow check for 16-bit system, for compatibility
       *      with FT_GlyphSlot_Embolden() since FreeType 2.1.10.
       *      unfortunately, this function return no informations
       *      about the cause of error.
       */
      if ( ( ystr >> 6 ) > FT_INT_MAX || ( ystr >> 6 ) < FT_INT_MIN )
      {
        FT_TRACE1(( "FT_GlyphSlot_Embolden:" ));
        FT_TRACE1(( "too strong emboldening parameter ystr=%ld\n", ystr ));
        return;
      }
      error = FT_GlyphSlot_Own_Bitmap( slot );
      if ( error )
        return;

      error = FT_Bitmap_WeightXY( library, &slot->bitmap, xstr, ystr );
      if ( error )
        return;
    }

    if ( slot->advance.x )
      slot->advance.x += xstr;

    if ( slot->advance.y )
      slot->advance.y += ystr;

    slot->metrics.width        += xstr;
    slot->metrics.height       += ystr;
    slot->metrics.horiAdvance  += xstr;
    slot->metrics.vertAdvance  += ystr;
    slot->metrics.horiBearingY += ystr;

    /* XXX: 16-bit overflow case must be excluded before here */
    if ( slot->format == FT_GLYPH_FORMAT_BITMAP )
      slot->bitmap_top += (FT_Int)( ystr >> 6 );
  }

/**
 TSIT }}}}}}}}}}
 */


/* END */
