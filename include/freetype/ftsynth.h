/****************************************************************************
 *
 * ftsynth.h
 *
 *   FreeType synthesizing code for emboldening and slanting
 *   (specification).
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


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*********                                                       *********/
  /*********        WARNING, THIS IS ALPHA CODE!  THIS API         *********/
  /*********    IS DUE TO CHANGE UNTIL STRICTLY NOTIFIED BY THE    *********/
  /*********            FREETYPE DEVELOPMENT TEAM                  *********/
  /*********                                                       *********/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /* Main reason for not lifting the functions in this module to a  */
  /* 'standard' API is that the used parameters for emboldening and */
  /* slanting are not configurable.  Consider the functions as a    */
  /* code resource that should be copied into the application and   */
  /* adapted to the particular needs.                               */


#ifndef FTSYNTH_H_
#define FTSYNTH_H_


#include <freetype/freetype.h>

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_TS_BEGIN_HEADER

  /* Embolden a glyph by a 'reasonable' value (which is highly a matter of */
  /* taste).  This function is actually a convenience function, providing  */
  /* a wrapper for @FT_TS_Outline_Embolden and @FT_TS_Bitmap_Embolden.           */
  /*                                                                       */
  /* For emboldened outlines the height, width, and advance metrics are    */
  /* increased by the strength of the emboldening -- this even affects     */
  /* mono-width fonts!                                                     */
  /*                                                                       */
  /* You can also call @FT_TS_Outline_Get_CBox to get precise values.         */
  FT_TS_EXPORT( void )
  FT_TS_GlyphSlot_Embolden( FT_TS_GlyphSlot  slot );

  /* Slant an outline glyph to the right by about 12 degrees. */
  FT_TS_EXPORT( void )
  FT_TS_GlyphSlot_Oblique( FT_TS_GlyphSlot  slot );

  /* */

/**
 TSIT {{{{{{{{{{
 */

  /**
   * Italic a slot for LTR or TTB
   */
  FT_TS_EXPORT( void )
  FT_TS_GlyphSlot_Oblique_Direction( FT_TS_GlyphSlot  slot, float oblique, int flags);

  /**
   * Transform a slot.
   */
  FT_TS_EXPORT( void )
  FT_TS_GlyphSlot_Transform( FT_TS_GlyphSlot  slot);

  /**
   * From pMatrix get degree.
   */
  FT_TS_EXPORT( int )
  FT_TS_GlyphSlot_Get_Matrix_Degree(FT_TS_Matrix* pMatrix);

  /**
   * Embolden or slim a slot.
   */
  FT_TS_EXPORT( void )
  FT_TS_GlyphSlot_Weight(   FT_TS_GlyphSlot  slot, float weight_x, float weight_y, int flags );

/**
 TSIT }}}}}}}}}}
 */

FT_TS_END_HEADER

#endif /* FTSYNTH_H_ */


/* END */
