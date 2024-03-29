/****************************************************************************
 *
 * otvgpos.h
 *
 *   OpenType GPOS table validator (specification).
 *
 * Copyright (C) 2004-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef OTVGPOS_H_
#define OTVGPOS_H_


FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( void )
  otv_GPOS_subtable_validate( FT_TS_Bytes       table,
                              OTV_Validator  valid );


FT_TS_END_HEADER

#endif /* OTVGPOS_H_ */


/* END */
