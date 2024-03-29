/****************************************************************************
 *
 * ftraster.h
 *
 *   The FreeType glyph rasterizer (specification).
 *
 * Copyright (C) 1996-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used
 * modified and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FTRASTER_H_
#define FTRASTER_H_


#include <ft2build.h>
#include FT_TS_CONFIG_CONFIG_H
#include <freetype/ftimage.h>

#include <freetype/internal/compiler-macros.h>

FT_TS_BEGIN_HEADER


  /**************************************************************************
   *
   * Uncomment the following line if you are using ftraster.c as a
   * standalone module, fully independent of FreeType.
   */
/* #define STANDALONE_ */

  FT_TS_EXPORT_VAR( const FT_TS_Raster_Funcs )  ft_standard_raster;


FT_TS_END_HEADER

#endif /* FTRASTER_H_ */


/* END */
