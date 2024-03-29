/****************************************************************************
 *
 * ttsvg.c
 *
 *   OpenType SVG Color (specification).
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


  /**************************************************************************
   *
   * 'SVG' table specification:
   *
   *    https://docs.microsoft.com/en-us/typography/opentype/spec/svg
   *
   */

#include <ft2build.h>
#include <freetype/internal/ftstream.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/tttags.h>
#include <freetype/ftgzip.h>
#include <freetype/otsvg.h>


#ifdef FT_TS_CONFIG_OPTION_SVG

#include "ttsvg.h"


  /* NOTE: These table sizes are given by the specification. */
#define SVG_TABLE_HEADER_SIZE           (10U)
#define SVG_DOCUMENT_RECORD_SIZE        (12U)
#define SVG_DOCUMENT_LIST_MINIMUM_SIZE  (2U + SVG_DOCUMENT_RECORD_SIZE)
#define SVG_MINIMUM_SIZE                (SVG_TABLE_HEADER_SIZE +        \
                                         SVG_DOCUMENT_LIST_MINIMUM_SIZE)


  typedef struct  Svg_
  {
    FT_TS_UShort  version;                 /* table version (starting at 0)  */
    FT_TS_UShort  num_entries;             /* number of SVG document records */

    FT_TS_Byte*  svg_doc_list;  /* pointer to the start of SVG Document List */

    void*     table;                          /* memory that backs up SVG */
    FT_TS_ULong  table_size;

  } Svg;


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, usued to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  ttsvg


  FT_TS_LOCAL_DEF( FT_TS_Error )
  tt_face_load_svg( TT_Face    face,
                    FT_TS_Stream  stream )
  {
    FT_TS_Error   error;
    FT_TS_Memory  memory = face->root.memory;

    FT_TS_ULong  table_size;
    FT_TS_Byte*  table = NULL;
    FT_TS_Byte*  p     = NULL;
    Svg*      svg   = NULL;
    FT_TS_ULong  offsetToSVGDocumentList;


    error = face->goto_table( face, TTAG_SVG, stream, &table_size );
    if ( error )
      goto NoSVG;

    if ( table_size < SVG_MINIMUM_SIZE )
      goto InvalidTable;

    if ( FT_TS_FRAME_EXTRACT( table_size, table ) )
      goto NoSVG;

    /* Allocate memory for the SVG object */
    if ( FT_TS_NEW( svg ) )
      goto NoSVG;

    p                       = table;
    svg->version            = FT_TS_NEXT_USHORT( p );
    offsetToSVGDocumentList = FT_TS_NEXT_ULONG( p );

    if ( offsetToSVGDocumentList < SVG_TABLE_HEADER_SIZE            ||
         offsetToSVGDocumentList > table_size -
                                     SVG_DOCUMENT_LIST_MINIMUM_SIZE )
      goto InvalidTable;

    svg->svg_doc_list = (FT_TS_Byte*)( table + offsetToSVGDocumentList );

    p                = svg->svg_doc_list;
    svg->num_entries = FT_TS_NEXT_USHORT( p );

    FT_TS_TRACE3(( "version: %d\n", svg->version ));
    FT_TS_TRACE3(( "number of entries: %d\n", svg->num_entries ));

    if ( offsetToSVGDocumentList +
           svg->num_entries * SVG_DOCUMENT_RECORD_SIZE > table_size )
      goto InvalidTable;

    svg->table      = table;
    svg->table_size = table_size;

    face->svg              = svg;
    face->root.face_flags |= FT_TS_FACE_FLAG_SVG;

    return FT_TS_Err_Ok;

  InvalidTable:
    error = FT_TS_THROW( Invalid_Table );

  NoSVG:
    FT_TS_FRAME_RELEASE( table );
    FT_TS_FREE( svg );
    face->svg = NULL;

    return error;
  }


  FT_TS_LOCAL_DEF( void )
  tt_face_free_svg( TT_Face  face )
  {
    FT_TS_Memory  memory = face->root.memory;
    FT_TS_Stream  stream = face->root.stream;

    Svg*  svg = (Svg*)face->svg;


    if ( svg )
    {
      FT_TS_FRAME_RELEASE( svg->table );
      FT_TS_FREE( svg );
    }
  }


  typedef struct  Svg_doc_
  {
    FT_TS_UShort  start_glyph_id;
    FT_TS_UShort  end_glyph_id;

    FT_TS_ULong  offset;
    FT_TS_ULong  length;

  } Svg_doc;


  static Svg_doc
  extract_svg_doc( FT_TS_Byte*  stream )
  {
    Svg_doc  doc;


    doc.start_glyph_id = FT_TS_NEXT_USHORT( stream );
    doc.end_glyph_id   = FT_TS_NEXT_USHORT( stream );

    doc.offset = FT_TS_NEXT_ULONG( stream );
    doc.length = FT_TS_NEXT_ULONG( stream );

    return doc;
  }


  static FT_TS_Int
  compare_svg_doc( Svg_doc  doc,
                   FT_TS_UInt  glyph_index )
  {
    if ( glyph_index < doc.start_glyph_id )
      return -1;
    else if ( glyph_index > doc.end_glyph_id )
      return 1;
    else
      return 0;
  }


  static FT_TS_Error
  find_doc( FT_TS_Byte*    stream,
            FT_TS_UShort   num_entries,
            FT_TS_UInt     glyph_index,
            FT_TS_ULong   *doc_offset,
            FT_TS_ULong   *doc_length,
            FT_TS_UShort  *start_glyph,
            FT_TS_UShort  *end_glyph )
  {
    FT_TS_Error  error;

    Svg_doc  start_doc;
    Svg_doc  mid_doc;
    Svg_doc  end_doc;

    FT_TS_Bool  found = FALSE;
    FT_TS_UInt  i     = 0;

    FT_TS_UInt  start_index = 0;
    FT_TS_UInt  end_index   = num_entries - 1;
    FT_TS_Int   comp_res;


    /* search algorithm */
    if ( num_entries == 0 )
    {
      error = FT_TS_THROW( Invalid_Table );
      return error;
    }

    start_doc = extract_svg_doc( stream + start_index * 12 );
    end_doc   = extract_svg_doc( stream + end_index * 12 );

    if ( ( compare_svg_doc( start_doc, glyph_index ) == -1 ) ||
         ( compare_svg_doc( end_doc, glyph_index ) == 1 )    )
    {
      error = FT_TS_THROW( Invalid_Glyph_Index );
      return error;
    }

    while ( start_index <= end_index )
    {
      i        = ( start_index + end_index ) / 2;
      mid_doc  = extract_svg_doc( stream + i * 12 );
      comp_res = compare_svg_doc( mid_doc, glyph_index );

      if ( comp_res == 1 )
      {
        start_index = i + 1;
        start_doc   = extract_svg_doc( stream + start_index * 4 );
      }
      else if ( comp_res == -1 )
      {
        end_index = i - 1;
        end_doc   = extract_svg_doc( stream + end_index * 4 );
      }
      else
      {
        found = TRUE;
        break;
      }
    }
    /* search algorithm end */

    if ( found != TRUE )
    {
      FT_TS_TRACE5(( "SVG glyph not found\n" ));
      error = FT_TS_THROW( Invalid_Glyph_Index );
    }
    else
    {
      *doc_offset = mid_doc.offset;
      *doc_length = mid_doc.length;

      *start_glyph = mid_doc.start_glyph_id;
      *end_glyph   = mid_doc.end_glyph_id;

      error = FT_TS_Err_Ok;
    }

    return error;
  }


  FT_TS_LOCAL_DEF( FT_TS_Error )
  tt_face_load_svg_doc( FT_TS_GlyphSlot  glyph,
                        FT_TS_UInt       glyph_index )
  {
    FT_TS_Byte*   doc_list;        /* pointer to the SVG doc list         */
    FT_TS_UShort  num_entries;     /* total number of entries in doc list */
    FT_TS_ULong   doc_offset;
    FT_TS_ULong   doc_length;

    FT_TS_UShort  start_glyph_id;
    FT_TS_UShort  end_glyph_id;

    FT_TS_Error   error  = FT_TS_Err_Ok;
    TT_Face    face   = (TT_Face)glyph->face;
    FT_TS_Memory  memory = face->root.memory;
    Svg*       svg    = (Svg*)face->svg;

    FT_TS_SVG_Document  svg_document = (FT_TS_SVG_Document)glyph->other;


    FT_TS_ASSERT( !( svg == NULL ) );

    doc_list    = svg->svg_doc_list;
    num_entries = FT_TS_NEXT_USHORT( doc_list );

    error = find_doc( doc_list, num_entries, glyph_index,
                                &doc_offset, &doc_length,
                                &start_glyph_id, &end_glyph_id );
    if ( error != FT_TS_Err_Ok )
      goto Exit;

    doc_list = svg->svg_doc_list;      /* reset, so we can use it again */
    doc_list = (FT_TS_Byte*)( doc_list + doc_offset );

    if ( ( doc_list[0] == 0x1F ) && ( doc_list[1] == 0x8B )
                                 && ( doc_list[2] == 0x08 ) )
    {
#ifdef FT_TS_CONFIG_OPTION_USE_ZLIB

      FT_TS_ULong  uncomp_size;
      FT_TS_Byte*  uncomp_buffer = NULL;


      /*
       * Get the size of the original document.  This helps in allotting the
       * buffer to accommodate the uncompressed version.  The last 4 bytes
       * of the compressed document are equal to the original size modulo
       * 2^32.  Since the size of SVG documents is less than 2^32 bytes we
       * can use this accurately.  The four bytes are stored in
       * little-endian format.
       */
      FT_TS_TRACE4(( "SVG document is GZIP compressed\n" ));
      uncomp_size = (FT_TS_ULong)doc_list[doc_length - 1] << 24 |
                    (FT_TS_ULong)doc_list[doc_length - 2] << 16 |
                    (FT_TS_ULong)doc_list[doc_length - 3] << 8  |
                    (FT_TS_ULong)doc_list[doc_length - 4];

      if ( FT_TS_QALLOC( uncomp_buffer, uncomp_size ) )
        goto Exit;

      error = FT_TS_Gzip_Uncompress( memory,
                                  uncomp_buffer,
                                  &uncomp_size,
                                  doc_list,
                                  doc_length );
      if ( error )
      {
        FT_TS_FREE( uncomp_buffer );
        error = FT_TS_THROW( Invalid_Table );
        goto Exit;
      }

      glyph->internal->flags |= FT_TS_GLYPH_OWN_GZIP_SVG;

      doc_list   = uncomp_buffer;
      doc_length = uncomp_size;

#else /* !FT_TS_CONFIG_OPTION_USE_ZLIB */

      error = FT_TS_THROW( Unimplemented_Feature );
      goto Exit;

#endif /* !FT_TS_CONFIG_OPTION_USE_ZLIB */
    }

    svg_document->svg_document        = doc_list;
    svg_document->svg_document_length = doc_length;

    svg_document->metrics      = glyph->face->size->metrics;
    svg_document->units_per_EM = glyph->face->units_per_EM;

    svg_document->start_glyph_id = start_glyph_id;
    svg_document->end_glyph_id   = end_glyph_id;

    svg_document->transform.xx = 0x10000;
    svg_document->transform.xy = 0;
    svg_document->transform.yx = 0;
    svg_document->transform.yy = 0x10000;

    svg_document->delta.x = 0;
    svg_document->delta.y = 0;

    FT_TS_TRACE5(( "start_glyph_id: %d\n", start_glyph_id ));
    FT_TS_TRACE5(( "end_glyph_id:   %d\n", end_glyph_id ));
    FT_TS_TRACE5(( "svg_document:\n" ));
    FT_TS_TRACE5(( " %.*s\n", (FT_TS_UInt)doc_length, doc_list ));

    glyph->other = svg_document;

  Exit:
    return error;
  }

#else /* !FT_TS_CONFIG_OPTION_SVG */

  /* ANSI C doesn't like empty source files */
  typedef int  _tt_svg_dummy;

#endif /* !FT_TS_CONFIG_OPTION_SVG */


/* END */
