/*
 * $Id: arraylist.c,v 1.4 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include <limits.h>

// #ifdef STDC_HEADERS
# include <stdlib.h>
# include <string.h>
// #endif /* STDC_HEADERS */

#if defined(HAVE_STRINGS_H) && !defined(_STRING_H) && !defined(__USE_BSD)
# include <strings.h>
#endif /* HAVE_STRINGS_H */

#ifndef SIZE_T_MAX
#if SIZEOF_SIZE_T == SIZEOF_INT
#define SIZE_T_MAX UINT_MAX
#elif SIZEOF_SIZE_T == SIZEOF_LONG
#define SIZE_T_MAX ULONG_MAX
#elif SIZEOF_SIZE_T == SIZEOF_LONG_LONG
#define SIZE_T_MAX ULLONG_MAX
#else
#error Unable to determine size of size_t
#endif
#endif

#include "array_list.h"

struct arraylist*
arraylist_new(arraylist_free_fn *free_fn)
{
  struct arraylist *arr;

  arr = (struct arraylist*)calloc(1, sizeof(struct arraylist));
  if(!arr) return NULL;
  arr->size = ARRAY_LIST_DEFAULT_SIZE;
  arr->length = 0;
  arr->free_fn = free_fn;
  if(!(arr->array = (void**)calloc(sizeof(void*), arr->size))) {
    free(arr);
    return NULL;
  }
  return arr;
}

extern void
arraylist_free(struct arraylist *arr)
{
  size_t i;
  for(i = 0; i < arr->length; i++)
    if(arr->array[i]) arr->free_fn(arr->array[i]);
  free(arr->array);
  free(arr);
}

void*
arraylist_get_idx(struct arraylist *arr, size_t i)
{
  if(i >= arr->length) return NULL;
  return arr->array[i];
}

static int arraylist_expand_internal(struct arraylist *arr, size_t max)
{
  void *t; // 重新分配后的指针
  size_t new_size; // 新的大小

  if(max < arr->size) return 0;
  /* Avoid undefined behaviour on size_t overflow */
  if( arr->size >= SIZE_T_MAX / 2 )
    new_size = max;
  else
  {
    new_size = arr->size << 1;
    if (new_size < max)
      new_size = max;
  }
  if (new_size > (~((size_t)0)) / sizeof(void*)) return -1;
  if (!(t = realloc(arr->array, new_size*sizeof(void*)))) return -1; // 阅读: if:如果, realloc:重新分配内存 , !: 不成功, return: 返回-1 
  arr->array = (void**)t;
  (void)memset(arr->array + arr->size, 0, (new_size-arr->size)*sizeof(void*));  // 初始化新扩展的内存为0
  arr->size = new_size;
  return 0;
}

int
arraylist_put_idx(struct arraylist *arr, size_t idx, void *data)
{
  if (idx > SIZE_T_MAX - 1 ) return -1;
  if(arraylist_expand_internal(arr, idx+1)) return -1;
  if(idx < arr->length && arr->array[idx])
    arr->free_fn(arr->array[idx]);
  arr->array[idx] = data;
  if(arr->length <= idx) arr->length = idx + 1;
  return 0;
}

int
arraylist_add(struct arraylist *arr, void *data)
{
  return arraylist_put_idx(arr, arr->length, data);
}

void
arraylist_sort(struct arraylist *arr, int(*compar)(const void *, const void *))
{
  qsort(arr->array, arr->length, sizeof(arr->array[0]), compar);
}

void* arraylist_bsearch(const void **key, struct arraylist *arr,
		int (*compar)(const void *, const void *))
{
	return bsearch(key, arr->array, arr->length, sizeof(arr->array[0]),
			compar);
}

size_t
arraylist_length(struct arraylist *arr)
{
  return arr->length;
}

int
arraylist_is_empty(struct arraylist *arr)
{
  return arr->length == 0 ? 1:0;
}

extern arraylist*
arraylist_sublist(struct arraylist *al, size_t start, size_t end)
{
  arraylist* list = NULL;
  for (size_t i = start; i < al->length && i < end; i++)
  {
    if(list==NULL){
      list = arraylist_new(al->free_fn);
    }
    arraylist_add(list, arraylist_get_idx(al,i));
  }
  return list;
}

int
arraylist_del_idx( struct arraylist *arr, size_t idx, size_t count )
{
	size_t i, stop;

	stop = idx + count;
	if ( idx >= arr->length || stop > arr->length ) return -1;
	for ( i = idx; i < stop; ++i ) {
		if ( arr->array[i] && arr->free_fn) 
      arr->free_fn( arr->array[i] );
	}
	memmove( arr->array + idx, arr->array + stop, (arr->length - stop) * sizeof(void*) );
	arr->length -= count;
	return 0;
}

extern void**
arraylist_toarray(struct arraylist *al)
{
  return al->array;
  // void** arr = calloc(al->length, sizeof(void*));
  // for (size_t i = 0; i < al->length; ++i ) {
  //   arr[i] = al->array[i];
  // }
  // return arr;
}

int arraylist_foreach(struct arraylist *arr, arraylist_iteration_fn f, void* context)
{
  for (size_t i = 0; i < arr->length; i++)
  {
    if(f(context, i, arr->array[i])){
      return i;
    }
  }
  return 0;
}