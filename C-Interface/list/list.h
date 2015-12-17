#ifndef LIST_INLCUDE_
#define LIST_INLCUDE_

typedef struct List_t {
	struct List_t *next;
	void *ptr;
} *List;

extern List list_new(void *x, ...);
extern List list_pop(List list, void **x);
extern List list_push(List list, void *x);
extern List list_copy(List list);
extern List list_reverse(List list);
extern List list_append(List list, List tail);
extern int  list_length(List list);
extern void list_free(List *list, char free_value);
extern void list_map(List list,\
		void apply(void **x, void *c), void *c);
extern void **list_to_array(List list, void *end);
#endif
