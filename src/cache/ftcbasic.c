/****************************************************************************
 *
 * ftcbasic.c
 *
 *   The FreeType basic cache interface (body).
 *
 * Copyright (C) 2003-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/ftcache.h>
#include "ftcglyph.h"
#include "ftcimage.h"
#include "ftcsbits.h"

#include "ftccback.h"
#include "ftcerror.h"

#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  cache


  /*
   * Basic Families
   *
   */
  typedef struct  FTC_BasicAttrRec_
  {
    FTC_ScalerRec  scaler;
    FT_TS_UInt        load_flags;

  } FTC_BasicAttrRec, *FTC_BasicAttrs;

#define FTC_BASIC_ATTR_COMPARE( a, b )                                 \
          FT_TS_BOOL( FTC_SCALER_COMPARE( &(a)->scaler, &(b)->scaler ) && \
                   (a)->load_flags == (b)->load_flags               )

#define FTC_BASIC_ATTR_HASH( a )                                     \
          ( FTC_SCALER_HASH( &(a)->scaler ) + 31 * (a)->load_flags )


  typedef struct  FTC_BasicQueryRec_
  {
    FTC_GQueryRec     gquery;
    FTC_BasicAttrRec  attrs;

  } FTC_BasicQueryRec, *FTC_BasicQuery;


  typedef struct  FTC_BasicFamilyRec_
  {
    FTC_FamilyRec     family;
    FTC_BasicAttrRec  attrs;

  } FTC_BasicFamilyRec, *FTC_BasicFamily;


  FT_TS_CALLBACK_DEF( FT_TS_Bool )
  ftc_basic_family_compare( FTC_MruNode  ftcfamily,
                            FT_TS_Pointer   ftcquery )
  {
    FTC_BasicFamily  family = (FTC_BasicFamily)ftcfamily;
    FTC_BasicQuery   query  = (FTC_BasicQuery)ftcquery;


    return FTC_BASIC_ATTR_COMPARE( &family->attrs, &query->attrs );
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ftc_basic_family_init( FTC_MruNode  ftcfamily,
                         FT_TS_Pointer   ftcquery,
                         FT_TS_Pointer   ftccache )
  {
    FTC_BasicFamily  family = (FTC_BasicFamily)ftcfamily;
    FTC_BasicQuery   query  = (FTC_BasicQuery)ftcquery;
    FTC_Cache        cache  = (FTC_Cache)ftccache;


    FTC_Family_Init( FTC_FAMILY( family ), cache );
    family->attrs = query->attrs;
    return 0;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt )
  ftc_basic_family_get_count( FTC_Family   ftcfamily,
                              FTC_Manager  manager )
  {
    FTC_BasicFamily  family = (FTC_BasicFamily)ftcfamily;
    FT_TS_Error         error;
    FT_TS_Face          face;
    FT_TS_UInt          result = 0;


    error = FTC_Manager_LookupFace( manager, family->attrs.scaler.face_id,
                                    &face );

    if ( error || !face )
      return result;

#ifdef FT_TS_DEBUG_LEVEL_TRACE
    if ( (FT_TS_ULong)face->num_glyphs > FT_TS_UINT_MAX || 0 > face->num_glyphs )
    {
      FT_TS_TRACE1(( "ftc_basic_family_get_count:"
                  " the number of glyphs in this face is %ld,\n",
                  face->num_glyphs ));
      FT_TS_TRACE1(( "                           "
                  " which is too much and thus truncated\n" ));
    }
#endif

    if ( !error )
      result = (FT_TS_UInt)face->num_glyphs;

    return result;
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ftc_basic_family_load_bitmap( FTC_Family   ftcfamily,
                                FT_TS_UInt      gindex,
                                FTC_Manager  manager,
                                FT_TS_Face     *aface )
  {
    FTC_BasicFamily  family = (FTC_BasicFamily)ftcfamily;
    FT_TS_Error         error;
    FT_TS_Size          size;


    error = FTC_Manager_LookupSize( manager, &family->attrs.scaler, &size );
    if ( !error )
    {
      FT_TS_Face  face = size->face;


      error = FT_TS_Load_Glyph(
                face,
                gindex,
                (FT_TS_Int)family->attrs.load_flags | FT_TS_LOAD_RENDER );
      if ( !error )
        *aface = face;
    }

    return error;
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ftc_basic_family_load_glyph( FTC_Family  ftcfamily,
                               FT_TS_UInt     gindex,
                               FTC_Cache   cache,
                               FT_TS_Glyph   *aglyph )
  {
    FTC_BasicFamily  family = (FTC_BasicFamily)ftcfamily;
    FT_TS_Error         error;
    FTC_Scaler       scaler = &family->attrs.scaler;
    FT_TS_Face          face;
    FT_TS_Size          size;


    /* we will now load the glyph image */
    error = FTC_Manager_LookupSize( cache->manager,
                                    scaler,
                                    &size );
    if ( !error )
    {
      face = size->face;

      error = FT_TS_Load_Glyph( face,
                             gindex,
                             (FT_TS_Int)family->attrs.load_flags );
      if ( !error )
      {
        if ( face->glyph->format == FT_TS_GLYPH_FORMAT_BITMAP  ||
             face->glyph->format == FT_TS_GLYPH_FORMAT_OUTLINE ||
             face->glyph->format == FT_TS_GLYPH_FORMAT_SVG     )
        {
          /* ok, copy it */
          FT_TS_Glyph  glyph;


          error = FT_TS_Get_Glyph( face->glyph, &glyph );
          if ( !error )
          {
            *aglyph = glyph;
            goto Exit;
          }
        }
        else
          error = FT_TS_THROW( Invalid_Argument );
      }
    }

  Exit:
    return error;
  }


  FT_TS_CALLBACK_DEF( FT_TS_Bool )
  ftc_basic_gnode_compare_faceid( FTC_Node    ftcgnode,
                                  FT_TS_Pointer  ftcface_id,
                                  FTC_Cache   cache,
                                  FT_TS_Bool*    list_changed )
  {
    FTC_GNode        gnode   = (FTC_GNode)ftcgnode;
    FTC_FaceID       face_id = (FTC_FaceID)ftcface_id;
    FTC_BasicFamily  family  = (FTC_BasicFamily)gnode->family;
    FT_TS_Bool          result;


    if ( list_changed )
      *list_changed = FALSE;
    result = FT_TS_BOOL( family->attrs.scaler.face_id == face_id );
    if ( result )
    {
      /* we must call this function to avoid this node from appearing
       * in later lookups with the same face_id!
       */
      FTC_GNode_UnselectFamily( gnode, cache );
    }
    return result;
  }


 /*
  *
  * basic image cache
  *
  */

  static
  const FTC_IFamilyClassRec  ftc_basic_image_family_class =
  {
    {
      sizeof ( FTC_BasicFamilyRec ),

      ftc_basic_family_compare, /* FTC_MruNode_CompareFunc  node_compare */
      ftc_basic_family_init,    /* FTC_MruNode_InitFunc     node_init    */
      NULL,                     /* FTC_MruNode_ResetFunc    node_reset   */
      NULL                      /* FTC_MruNode_DoneFunc     node_done    */
    },

    ftc_basic_family_load_glyph /* FTC_IFamily_LoadGlyphFunc  family_load_glyph */
  };


  static
  const FTC_GCacheClassRec  ftc_basic_image_cache_class =
  {
    {
      ftc_inode_new,                  /* FTC_Node_NewFunc      node_new           */
      ftc_inode_weight,               /* FTC_Node_WeightFunc   node_weight        */
      ftc_gnode_compare,              /* FTC_Node_CompareFunc  node_compare       */
      ftc_basic_gnode_compare_faceid, /* FTC_Node_CompareFunc  node_remove_faceid */
      ftc_inode_free,                 /* FTC_Node_FreeFunc     node_free          */

      sizeof ( FTC_GCacheRec ),
      ftc_gcache_init,                /* FTC_Cache_InitFunc    cache_init         */
      ftc_gcache_done                 /* FTC_Cache_DoneFunc    cache_done         */
    },

    (FTC_MruListClass)&ftc_basic_image_family_class
  };


  /* documentation is in ftcache.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FTC_ImageCache_New( FTC_Manager      manager,
                      FTC_ImageCache  *acache )
  {
    return FTC_GCache_New( manager, &ftc_basic_image_cache_class,
                           (FTC_GCache*)acache );
  }


  /* documentation is in ftcache.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FTC_ImageCache_Lookup( FTC_ImageCache  cache,
                         FTC_ImageType   type,
                         FT_TS_UInt         gindex,
                         FT_TS_Glyph       *aglyph,
                         FTC_Node       *anode )
  {
    FTC_BasicQueryRec  query;
    FTC_Node           node = 0; /* make compiler happy */
    FT_TS_Error           error;
    FT_TS_Offset          hash;


    /* some argument checks are delayed to `FTC_Cache_Lookup' */
    if ( !aglyph )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    *aglyph = NULL;
    if ( anode )
      *anode  = NULL;

    /*
     * Internal `FTC_BasicAttr->load_flags' is of type `FT_TS_UInt',
     * but public `FT_TS_ImageType->flags' is of type `FT_TS_Int32'.
     *
     * On 16bit systems, higher bits of type->flags cannot be handled.
     */
#if 0xFFFFFFFFUL > FT_TS_UINT_MAX
    if ( (type->flags & (FT_TS_ULong)FT_TS_UINT_MAX) )
      FT_TS_TRACE1(( "FTC_ImageCache_Lookup:"
                  " higher bits in load_flags 0x%x are dropped\n",
                  (FT_TS_ULong)type->flags & ~((FT_TS_ULong)FT_TS_UINT_MAX) ));
#endif

    query.attrs.scaler.face_id = type->face_id;
    query.attrs.scaler.width   = type->width;
    query.attrs.scaler.height  = type->height;
    query.attrs.load_flags     = (FT_TS_UInt)type->flags;

    query.attrs.scaler.pixel = 1;
    query.attrs.scaler.x_res = 0;  /* make compilers happy */
    query.attrs.scaler.y_res = 0;

    hash = FTC_BASIC_ATTR_HASH( &query.attrs ) + gindex;

#if 1  /* inlining is about 50% faster! */
    FTC_GCACHE_LOOKUP_CMP( cache,
                           ftc_basic_family_compare,
                           FTC_GNode_Compare,
                           hash, gindex,
                           &query,
                           node,
                           error );
#else
    error = FTC_GCache_Lookup( FTC_GCACHE( cache ),
                               hash, gindex,
                               FTC_GQUERY( &query ),
                               &node );
#endif
    if ( !error )
    {
      *aglyph = FTC_INODE( node )->glyph;

      if ( anode )
      {
        *anode = node;
        node->ref_count++;
      }
    }

  Exit:
    return error;
  }


  /* documentation is in ftcache.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FTC_ImageCache_LookupScaler( FTC_ImageCache  cache,
                               FTC_Scaler      scaler,
                               FT_TS_ULong        load_flags,
                               FT_TS_UInt         gindex,
                               FT_TS_Glyph       *aglyph,
                               FTC_Node       *anode )
  {
    FTC_BasicQueryRec  query;
    FTC_Node           node = 0; /* make compiler happy */
    FT_TS_Error           error;
    FT_TS_Offset          hash;


    /* some argument checks are delayed to `FTC_Cache_Lookup' */
    if ( !aglyph || !scaler )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    *aglyph = NULL;
    if ( anode )
      *anode  = NULL;

    /*
     * Internal `FTC_BasicAttr->load_flags' is of type `FT_TS_UInt',
     * but public `FT_TS_Face->face_flags' is of type `FT_TS_Long'.
     *
     * On long > int systems, higher bits of load_flags cannot be handled.
     */
#if FT_TS_ULONG_MAX > FT_TS_UINT_MAX
    if ( load_flags > FT_TS_UINT_MAX )
      FT_TS_TRACE1(( "FTC_ImageCache_LookupScaler:"
                  " higher bits in load_flags 0x%lx are dropped\n",
                  load_flags & ~((FT_TS_ULong)FT_TS_UINT_MAX) ));
#endif

    query.attrs.scaler     = scaler[0];
    query.attrs.load_flags = (FT_TS_UInt)load_flags;

    hash = FTC_BASIC_ATTR_HASH( &query.attrs ) + gindex;

    FTC_GCACHE_LOOKUP_CMP( cache,
                           ftc_basic_family_compare,
                           FTC_GNode_Compare,
                           hash, gindex,
                           &query,
                           node,
                           error );
    if ( !error )
    {
      *aglyph = FTC_INODE( node )->glyph;

      if ( anode )
      {
        *anode = node;
        node->ref_count++;
      }
    }

  Exit:
    return error;
  }


  /*
   *
   * basic small bitmap cache
   *
   */

  static
  const FTC_SFamilyClassRec  ftc_basic_sbit_family_class =
  {
    {
      sizeof ( FTC_BasicFamilyRec ),
      ftc_basic_family_compare,     /* FTC_MruNode_CompareFunc  node_compare */
      ftc_basic_family_init,        /* FTC_MruNode_InitFunc     node_init    */
      NULL,                         /* FTC_MruNode_ResetFunc    node_reset   */
      NULL                          /* FTC_MruNode_DoneFunc     node_done    */
    },

    ftc_basic_family_get_count,
    ftc_basic_family_load_bitmap
  };


  static
  const FTC_GCacheClassRec  ftc_basic_sbit_cache_class =
  {
    {
      ftc_snode_new,                  /* FTC_Node_NewFunc      node_new           */
      ftc_snode_weight,               /* FTC_Node_WeightFunc   node_weight        */
      ftc_snode_compare,              /* FTC_Node_CompareFunc  node_compare       */
      ftc_basic_gnode_compare_faceid, /* FTC_Node_CompareFunc  node_remove_faceid */
      ftc_snode_free,                 /* FTC_Node_FreeFunc     node_free          */

      sizeof ( FTC_GCacheRec ),
      ftc_gcache_init,                /* FTC_Cache_InitFunc    cache_init         */
      ftc_gcache_done                 /* FTC_Cache_DoneFunc    cache_done         */
    },

    (FTC_MruListClass)&ftc_basic_sbit_family_class
  };


  /* documentation is in ftcache.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FTC_SBitCache_New( FTC_Manager     manager,
                     FTC_SBitCache  *acache )
  {
    return FTC_GCache_New( manager, &ftc_basic_sbit_cache_class,
                           (FTC_GCache*)acache );
  }


  /* documentation is in ftcache.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FTC_SBitCache_Lookup( FTC_SBitCache  cache,
                        FTC_ImageType  type,
                        FT_TS_UInt        gindex,
                        FTC_SBit      *ansbit,
                        FTC_Node      *anode )
  {
    FT_TS_Error           error;
    FTC_BasicQueryRec  query;
    FTC_Node           node = 0; /* make compiler happy */
    FT_TS_Offset          hash;


    if ( anode )
      *anode = NULL;

    /* other argument checks delayed to `FTC_Cache_Lookup' */
    if ( !ansbit )
      return FT_TS_THROW( Invalid_Argument );

    *ansbit = NULL;

    /*
     * Internal `FTC_BasicAttr->load_flags' is of type `FT_TS_UInt',
     * but public `FT_TS_ImageType->flags' is of type `FT_TS_Int32'.
     *
     * On 16bit systems, higher bits of type->flags cannot be handled.
     */
#if 0xFFFFFFFFUL > FT_TS_UINT_MAX
    if ( (type->flags & (FT_TS_ULong)FT_TS_UINT_MAX) )
      FT_TS_TRACE1(( "FTC_ImageCache_Lookup:"
                  " higher bits in load_flags 0x%x are dropped\n",
                  (FT_TS_ULong)type->flags & ~((FT_TS_ULong)FT_TS_UINT_MAX) ));
#endif

    query.attrs.scaler.face_id = type->face_id;
    query.attrs.scaler.width   = type->width;
    query.attrs.scaler.height  = type->height;
    query.attrs.load_flags     = (FT_TS_UInt)type->flags;

    query.attrs.scaler.pixel = 1;
    query.attrs.scaler.x_res = 0;  /* make compilers happy */
    query.attrs.scaler.y_res = 0;

    /* beware, the hash must be the same for all glyph ranges! */
    hash = FTC_BASIC_ATTR_HASH( &query.attrs ) +
           gindex / FTC_SBIT_ITEMS_PER_NODE;

#if 1  /* inlining is about 50% faster! */
    FTC_GCACHE_LOOKUP_CMP( cache,
                           ftc_basic_family_compare,
                           FTC_SNode_Compare,
                           hash, gindex,
                           &query,
                           node,
                           error );
#else
    error = FTC_GCache_Lookup( FTC_GCACHE( cache ),
                               hash,
                               gindex,
                               FTC_GQUERY( &query ),
                               &node );
#endif
    if ( error )
      goto Exit;

    *ansbit = FTC_SNODE( node )->sbits +
              ( gindex - FTC_GNODE( node )->gindex );

    if ( anode )
    {
      *anode = node;
      node->ref_count++;
    }

  Exit:
    return error;
  }


  /* documentation is in ftcache.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FTC_SBitCache_LookupScaler( FTC_SBitCache  cache,
                              FTC_Scaler     scaler,
                              FT_TS_ULong       load_flags,
                              FT_TS_UInt        gindex,
                              FTC_SBit      *ansbit,
                              FTC_Node      *anode )
  {
    FT_TS_Error           error;
    FTC_BasicQueryRec  query;
    FTC_Node           node = 0; /* make compiler happy */
    FT_TS_Offset          hash;


    if ( anode )
        *anode = NULL;

    /* other argument checks delayed to `FTC_Cache_Lookup' */
    if ( !ansbit || !scaler )
        return FT_TS_THROW( Invalid_Argument );

    *ansbit = NULL;

    /*
     * Internal `FTC_BasicAttr->load_flags' is of type `FT_TS_UInt',
     * but public `FT_TS_Face->face_flags' is of type `FT_TS_Long'.
     *
     * On long > int systems, higher bits of load_flags cannot be handled.
     */
#if FT_TS_ULONG_MAX > FT_TS_UINT_MAX
    if ( load_flags > FT_TS_UINT_MAX )
      FT_TS_TRACE1(( "FTC_ImageCache_LookupScaler:"
                  " higher bits in load_flags 0x%lx are dropped\n",
                  load_flags & ~((FT_TS_ULong)FT_TS_UINT_MAX) ));
#endif

    query.attrs.scaler     = scaler[0];
    query.attrs.load_flags = (FT_TS_UInt)load_flags;

    /* beware, the hash must be the same for all glyph ranges! */
    hash = FTC_BASIC_ATTR_HASH( &query.attrs ) +
             gindex / FTC_SBIT_ITEMS_PER_NODE;

    FTC_GCACHE_LOOKUP_CMP( cache,
                           ftc_basic_family_compare,
                           FTC_SNode_Compare,
                           hash, gindex,
                           &query,
                           node,
                           error );
    if ( error )
      goto Exit;

    *ansbit = FTC_SNODE( node )->sbits +
              ( gindex - FTC_GNODE( node )->gindex );

    if ( anode )
    {
      *anode = node;
      node->ref_count++;
    }

  Exit:
    return error;
  }


/* END */
