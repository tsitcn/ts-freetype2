/****************************************************************************
 *
 * pfrsbit.h
 *
 *   FreeType PFR bitmap loader (specification).
 *
 * Copyright (C) 2002-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef PFRSBIT_H_
#define PFRSBIT_H_

#include "pfrobjs.h"

FT_TS_BEGIN_HEADER

  FT_TS_LOCAL( FT_TS_Error )
  pfr_slot_load_bitmap( PFR_Slot  glyph,
                        PFR_Size  size,
                        FT_TS_UInt   glyph_index,
                        FT_TS_Bool   metrics_only );

FT_TS_END_HEADER

#endif /* PFRSBIT_H_ */


/* END */
