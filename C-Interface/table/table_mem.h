#ifndef TABLE_H_
#define TABLE_H_

typedef struct table_t *table_t;
typedef int (*comparator)(const void *x, const void *y); /*定义一个比较器类型*/
typedef unsigned int (*hashfuc)(const void *key);/*定义一个hash函数类型*/

extern struct table_t *table_new(int sz, comparator cp, hashfuc hs);
extern void *table_put(struct table_t *tb, const void *key, void *value);
extern void *table_get(struct table_t *tb, const void *key);
extern unsigned int table_length(struct table_t *tb);
extern void *table_remove(struct table_t *tb, const void *key);
extern void table_map(struct table_t *tb,\
			   void apply(const void *key, void **val, void *cp),\
			   void *cp);
extern void **table_to_array(struct table_t *tb, void *end);
extern void table_free(struct table_t *tb);

#endif
