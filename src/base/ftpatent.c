/****************************************************************************
 *
 * ftpatent.c
 *
 *   FreeType API for checking patented TrueType bytecode instructions
 *   (body).  Obsolete, retained for backward compatibility.
 *
 * Copyright (C) 2007-2022 by
 * David Turner.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#include <freetype/freetype.h>
#include <freetype/tttags.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftstream.h>
#include <freetype/internal/services/svsfnt.h>
#include <freetype/internal/services/svttglyf.h>


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Bool )
  FT_TS_Face_CheckTrueTypePatents( FT_TS_Face  face )
  {
    FT_TS_UNUSED( face );

    return FALSE;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Bool )
  FT_TS_Face_SetUnpatentedHinting( FT_TS_Face  face,
                                FT_TS_Bool  value )
  {
    FT_TS_UNUSED( face );
    FT_TS_UNUSED( value );

    return FALSE;
  }

/* END */
