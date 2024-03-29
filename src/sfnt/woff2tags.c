/****************************************************************************
 *
 * woff2tags.c
 *
 *   WOFF2 Font table tags (base).
 *
 * Copyright (C) 2019-2022 by
 * Nikhil Ramakrishnan, David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include <freetype/tttags.h>

#ifdef FT_TS_CONFIG_OPTION_USE_BROTLI

#include "woff2tags.h"

  /*
   * Return tag from index in the order given in WOFF2 specification.
   *
   * See
   *
   *       https://www.w3.org/TR/WOFF2/#table_dir_format
   *
   * for details.
   */
  FT_TS_LOCAL_DEF( FT_TS_Tag )
  woff2_known_tags( FT_TS_Byte  index )
  {
    static const FT_TS_Tag  known_tags[63] =
    {
      FT_TS_MAKE_TAG('c', 'm', 'a', 'p'),  /*  0  */
      FT_TS_MAKE_TAG('h', 'e', 'a', 'd'),  /*  1  */
      FT_TS_MAKE_TAG('h', 'h', 'e', 'a'),  /*  2  */
      FT_TS_MAKE_TAG('h', 'm', 't', 'x'),  /*  3  */
      FT_TS_MAKE_TAG('m', 'a', 'x', 'p'),  /*  4  */
      FT_TS_MAKE_TAG('n', 'a', 'm', 'e'),  /*  5  */
      FT_TS_MAKE_TAG('O', 'S', '/', '2'),  /*  6  */
      FT_TS_MAKE_TAG('p', 'o', 's', 't'),  /*  7  */
      FT_TS_MAKE_TAG('c', 'v', 't', ' '),  /*  8  */
      FT_TS_MAKE_TAG('f', 'p', 'g', 'm'),  /*  9  */
      FT_TS_MAKE_TAG('g', 'l', 'y', 'f'),  /*  10 */
      FT_TS_MAKE_TAG('l', 'o', 'c', 'a'),  /*  11 */
      FT_TS_MAKE_TAG('p', 'r', 'e', 'p'),  /*  12 */
      FT_TS_MAKE_TAG('C', 'F', 'F', ' '),  /*  13 */
      FT_TS_MAKE_TAG('V', 'O', 'R', 'G'),  /*  14 */
      FT_TS_MAKE_TAG('E', 'B', 'D', 'T'),  /*  15 */
      FT_TS_MAKE_TAG('E', 'B', 'L', 'C'),  /*  16 */
      FT_TS_MAKE_TAG('g', 'a', 's', 'p'),  /*  17 */
      FT_TS_MAKE_TAG('h', 'd', 'm', 'x'),  /*  18 */
      FT_TS_MAKE_TAG('k', 'e', 'r', 'n'),  /*  19 */
      FT_TS_MAKE_TAG('L', 'T', 'S', 'H'),  /*  20 */
      FT_TS_MAKE_TAG('P', 'C', 'L', 'T'),  /*  21 */
      FT_TS_MAKE_TAG('V', 'D', 'M', 'X'),  /*  22 */
      FT_TS_MAKE_TAG('v', 'h', 'e', 'a'),  /*  23 */
      FT_TS_MAKE_TAG('v', 'm', 't', 'x'),  /*  24 */
      FT_TS_MAKE_TAG('B', 'A', 'S', 'E'),  /*  25 */
      FT_TS_MAKE_TAG('G', 'D', 'E', 'F'),  /*  26 */
      FT_TS_MAKE_TAG('G', 'P', 'O', 'S'),  /*  27 */
      FT_TS_MAKE_TAG('G', 'S', 'U', 'B'),  /*  28 */
      FT_TS_MAKE_TAG('E', 'B', 'S', 'C'),  /*  29 */
      FT_TS_MAKE_TAG('J', 'S', 'T', 'F'),  /*  30 */
      FT_TS_MAKE_TAG('M', 'A', 'T', 'H'),  /*  31 */
      FT_TS_MAKE_TAG('C', 'B', 'D', 'T'),  /*  32 */
      FT_TS_MAKE_TAG('C', 'B', 'L', 'C'),  /*  33 */
      FT_TS_MAKE_TAG('C', 'O', 'L', 'R'),  /*  34 */
      FT_TS_MAKE_TAG('C', 'P', 'A', 'L'),  /*  35 */
      FT_TS_MAKE_TAG('S', 'V', 'G', ' '),  /*  36 */
      FT_TS_MAKE_TAG('s', 'b', 'i', 'x'),  /*  37 */
      FT_TS_MAKE_TAG('a', 'c', 'n', 't'),  /*  38 */
      FT_TS_MAKE_TAG('a', 'v', 'a', 'r'),  /*  39 */
      FT_TS_MAKE_TAG('b', 'd', 'a', 't'),  /*  40 */
      FT_TS_MAKE_TAG('b', 'l', 'o', 'c'),  /*  41 */
      FT_TS_MAKE_TAG('b', 's', 'l', 'n'),  /*  42 */
      FT_TS_MAKE_TAG('c', 'v', 'a', 'r'),  /*  43 */
      FT_TS_MAKE_TAG('f', 'd', 's', 'c'),  /*  44 */
      FT_TS_MAKE_TAG('f', 'e', 'a', 't'),  /*  45 */
      FT_TS_MAKE_TAG('f', 'm', 't', 'x'),  /*  46 */
      FT_TS_MAKE_TAG('f', 'v', 'a', 'r'),  /*  47 */
      FT_TS_MAKE_TAG('g', 'v', 'a', 'r'),  /*  48 */
      FT_TS_MAKE_TAG('h', 's', 't', 'y'),  /*  49 */
      FT_TS_MAKE_TAG('j', 'u', 's', 't'),  /*  50 */
      FT_TS_MAKE_TAG('l', 'c', 'a', 'r'),  /*  51 */
      FT_TS_MAKE_TAG('m', 'o', 'r', 't'),  /*  52 */
      FT_TS_MAKE_TAG('m', 'o', 'r', 'x'),  /*  53 */
      FT_TS_MAKE_TAG('o', 'p', 'b', 'd'),  /*  54 */
      FT_TS_MAKE_TAG('p', 'r', 'o', 'p'),  /*  55 */
      FT_TS_MAKE_TAG('t', 'r', 'a', 'k'),  /*  56 */
      FT_TS_MAKE_TAG('Z', 'a', 'p', 'f'),  /*  57 */
      FT_TS_MAKE_TAG('S', 'i', 'l', 'f'),  /*  58 */
      FT_TS_MAKE_TAG('G', 'l', 'a', 't'),  /*  59 */
      FT_TS_MAKE_TAG('G', 'l', 'o', 'c'),  /*  60 */
      FT_TS_MAKE_TAG('F', 'e', 'a', 't'),  /*  61 */
      FT_TS_MAKE_TAG('S', 'i', 'l', 'l'),  /*  62 */
    };


    if ( index > 62 )
      return 0;

    return known_tags[index];
  }

#else /* !FT_TS_CONFIG_OPTION_USE_BROTLI */

  /* ANSI C doesn't like empty source files */
  typedef int  _woff2tags_dummy;

#endif /* !FT_TS_CONFIG_OPTION_USE_BROTLI */


/* END */
