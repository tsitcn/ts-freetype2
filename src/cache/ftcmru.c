/****************************************************************************
 *
 * ftcmru.c
 *
 *   FreeType MRU support (body).
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


#include <freetype/ftcache.h>
#include "ftcmru.h"
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>

#include "ftcerror.h"


  FT_TS_LOCAL_DEF( void )
  FTC_MruNode_Prepend( FTC_MruNode  *plist,
                       FTC_MruNode   node )
  {
    FTC_MruNode  first = *plist;


    if ( first )
    {
      FTC_MruNode  last = first->prev;


#ifdef FT_TS_DEBUG_ERROR
      {
        FTC_MruNode  cnode = first;


        do
        {
          if ( cnode == node )
          {
            fprintf( stderr, "FTC_MruNode_Prepend: invalid action\n" );
            exit( 2 );
          }
          cnode = cnode->next;

        } while ( cnode != first );
      }
#endif

      first->prev = node;
      last->next  = node;
      node->next  = first;
      node->prev  = last;
    }
    else
    {
      node->next = node;
      node->prev = node;
    }
    *plist = node;
  }


  FT_TS_LOCAL_DEF( void )
  FTC_MruNode_Up( FTC_MruNode  *plist,
                  FTC_MruNode   node )
  {
    FTC_MruNode  first = *plist;


    FT_TS_ASSERT( first );

    if ( first != node )
    {
      FTC_MruNode  prev, next, last;


#ifdef FT_TS_DEBUG_ERROR
      {
        FTC_MruNode  cnode = first;
        do
        {
          if ( cnode == node )
            goto Ok;
          cnode = cnode->next;

        } while ( cnode != first );

        fprintf( stderr, "FTC_MruNode_Up: invalid action\n" );
        exit( 2 );
      Ok:
      }
#endif
      prev = node->prev;
      next = node->next;

      prev->next = next;
      next->prev = prev;

      last = first->prev;

      last->next  = node;
      first->prev = node;

      node->next = first;
      node->prev = last;

      *plist = node;
    }
  }


  FT_TS_LOCAL_DEF( void )
  FTC_MruNode_Remove( FTC_MruNode  *plist,
                      FTC_MruNode   node )
  {
    FTC_MruNode  first = *plist;
    FTC_MruNode  prev, next;


    FT_TS_ASSERT( first );

#ifdef FT_TS_DEBUG_ERROR
      {
        FTC_MruNode  cnode = first;


        do
        {
          if ( cnode == node )
            goto Ok;
          cnode = cnode->next;

        } while ( cnode != first );

        fprintf( stderr, "FTC_MruNode_Remove: invalid action\n" );
        exit( 2 );
      Ok:
      }
#endif

    prev = node->prev;
    next = node->next;

    prev->next = next;
    next->prev = prev;

    if ( node == next )
    {
      FT_TS_ASSERT( first == node );
      FT_TS_ASSERT( prev  == node );

      *plist = NULL;
    }
    else if ( node == first )
      *plist = next;
  }


  FT_TS_LOCAL_DEF( void )
  FTC_MruList_Init( FTC_MruList       list,
                    FTC_MruListClass  clazz,
                    FT_TS_UInt           max_nodes,
                    FT_TS_Pointer        data,
                    FT_TS_Memory         memory )
  {
    list->num_nodes = 0;
    list->max_nodes = max_nodes;
    list->nodes     = NULL;
    list->clazz     = *clazz;
    list->data      = data;
    list->memory    = memory;
  }


  FT_TS_LOCAL_DEF( void )
  FTC_MruList_Reset( FTC_MruList  list )
  {
    while ( list->nodes )
      FTC_MruList_Remove( list, list->nodes );

    FT_TS_ASSERT( list->num_nodes == 0 );
  }


  FT_TS_LOCAL_DEF( void )
  FTC_MruList_Done( FTC_MruList  list )
  {
    FTC_MruList_Reset( list );
  }


#ifndef FTC_INLINE
  FT_TS_LOCAL_DEF( FTC_MruNode )
  FTC_MruList_Find( FTC_MruList  list,
                    FT_TS_Pointer   key )
  {
    FTC_MruNode_CompareFunc  compare = list->clazz.node_compare;
    FTC_MruNode              first, node;


    first = list->nodes;
    node  = NULL;

    if ( first )
    {
      node = first;
      do
      {
        if ( compare( node, key ) )
        {
          if ( node != first )
            FTC_MruNode_Up( &list->nodes, node );

          return node;
        }

        node = node->next;

      } while ( node != first);
    }

    return NULL;
  }
#endif

  FT_TS_LOCAL_DEF( FT_TS_Error )
  FTC_MruList_New( FTC_MruList   list,
                   FT_TS_Pointer    key,
                   FTC_MruNode  *anode )
  {
    FT_TS_Error     error;
    FTC_MruNode  node   = NULL;
    FT_TS_Memory    memory = list->memory;


    if ( list->num_nodes >= list->max_nodes && list->max_nodes > 0 )
    {
      node = list->nodes->prev;

      FT_TS_ASSERT( node );

      if ( list->clazz.node_reset )
      {
        FTC_MruNode_Up( &list->nodes, node );

        error = list->clazz.node_reset( node, key, list->data );
        if ( !error )
          goto Exit;
      }

      FTC_MruNode_Remove( &list->nodes, node );
      list->num_nodes--;

      if ( list->clazz.node_done )
        list->clazz.node_done( node, list->data );
    }

    /* zero new node in case of node_init failure */
    else if ( FT_TS_ALLOC( node, list->clazz.node_size ) )
      goto Exit;

    error = list->clazz.node_init( node, key, list->data );
    if ( error )
      goto Fail;

    FTC_MruNode_Prepend( &list->nodes, node );
    list->num_nodes++;

  Exit:
    *anode = node;
    return error;

  Fail:
    if ( list->clazz.node_done )
      list->clazz.node_done( node, list->data );

    FT_TS_FREE( node );
    goto Exit;
  }


#ifndef FTC_INLINE
  FT_TS_LOCAL_DEF( FT_TS_Error )
  FTC_MruList_Lookup( FTC_MruList   list,
                      FT_TS_Pointer    key,
                      FTC_MruNode  *anode )
  {
    FTC_MruNode  node;


    node = FTC_MruList_Find( list, key );
    if ( !node )
      return FTC_MruList_New( list, key, anode );

    *anode = node;
    return 0;
  }
#endif /* FTC_INLINE */

  FT_TS_LOCAL_DEF( void )
  FTC_MruList_Remove( FTC_MruList  list,
                      FTC_MruNode  node )
  {
    FTC_MruNode_Remove( &list->nodes, node );
    list->num_nodes--;

    {
      FT_TS_Memory  memory = list->memory;


      if ( list->clazz.node_done )
        list->clazz.node_done( node, list->data );

      FT_TS_FREE( node );
    }
  }


  FT_TS_LOCAL_DEF( void )
  FTC_MruList_RemoveSelection( FTC_MruList              list,
                               FTC_MruNode_CompareFunc  selection,
                               FT_TS_Pointer               key )
  {
    FTC_MruNode  first, node, next;


    first = list->nodes;
    while ( first && ( !selection || selection( first, key ) ) )
    {
      FTC_MruList_Remove( list, first );
      first = list->nodes;
    }

    if ( first )
    {
      node = first->next;
      while ( node != first )
      {
        next = node->next;

        if ( selection( node, key ) )
          FTC_MruList_Remove( list, node );

        node = next;
      }
    }
  }


/* END */
