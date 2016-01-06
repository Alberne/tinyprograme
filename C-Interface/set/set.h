#ifndef SET_H_
#define SET_H_

struct set_t;
typedef int (*comparator)(const void *x, const void *y); /*定义一个比较器类型*/
typedef unsigned int (*hashfuc)(const void *key);/*定义一个hash函数类型*/

extern struct set_t *set_new(int sz, comparator cp, hashfuc hs);
extern int set_member(struct set_t *set, const void *member);
extern void set_put(struct set_t *set, const void *member);
extern int set_remove(struct set_t *set, const void *member);
extern int set_length(struct set_t *set);
extern void set_free(struct set_t **pset);
extern void set_map(struct set_t *set,\
				void apply(const void *member, void *c),\
				void *c);
extern void **set_toarray(struct set_t *set, const void *end);
extern struct set_t *set_union(struct set_t *t, struct set_t *s);
extern struct set_t *set_inter(struct set_t *s, struct set_t *t);
extern struct set_t *set_minus(struct set_t *t, struct set_t *s);
extern struct set_t *set_diff(struct set_t *t, struct set_t *s);

				

#endif