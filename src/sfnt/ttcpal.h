/****************************************************************************
 *
 * ttcpal.h
 *
 *   TrueType and OpenType color palette support (specification).
 *
 * Copyright (C) 2018-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * Originally written by Shao Yu Zhang <shaozhang@fb.com>.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef __TTCPAL_H__
#define __TTCPAL_H__


#include "ttload.h"


FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( FT_TS_Error )
  tt_face_load_cpal( TT_Face    face,
                     FT_TS_Stream  stream );

  FT_TS_LOCAL( void )
  tt_face_free_cpal( TT_Face  face );

  FT_TS_LOCAL( FT_TS_Error )
  tt_face_palette_set( TT_Face  face,
                       FT_TS_UInt  palette_index );


FT_TS_END_HEADER


#endif /* __TTCPAL_H__ */

/* END */
