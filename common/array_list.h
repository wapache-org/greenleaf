#ifndef _ARRAY_LIST_H_
#define _ARRAY_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ARRAY_LIST_DEFAULT_SIZE 32

typedef void (arraylist_free_fn) (void *data);

struct arraylist
{
  void **array; // void* array[];
  size_t length; // item count
  size_t size;  // capital, array[]'s size
  arraylist_free_fn *free_fn;
};
typedef struct arraylist arraylist;

extern struct arraylist*
arraylist_new(arraylist_free_fn *free_fn);

extern void
arraylist_free(struct arraylist *al);

extern void*
arraylist_get_idx(struct arraylist *al, size_t i);

extern int
arraylist_put_idx(struct arraylist *al, size_t i, void *data);

extern int
arraylist_add(struct arraylist *al, void *data);

extern size_t
arraylist_length(struct arraylist *al);

extern int
arraylist_is_empty(struct arraylist *al);

extern arraylist*
arraylist_sublist(struct arraylist *al, size_t start, size_t end);

extern void**
arraylist_toarray(struct arraylist *al);

extern void
arraylist_sort(struct arraylist *arr, int(*compar)(const void *, const void *));

extern void*
arraylist_bsearch(const void **key, struct arraylist *arr,
		int (*compar)(const void *, const void *));

extern int 
arraylist_del_idx(struct arraylist *arr, size_t idx, size_t count);

#ifdef __cplusplus
}
#endif

#endif // _ARRAY_LIST_H_
