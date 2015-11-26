#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include "table.h"

static int cmpdef(const void *x, const void *y)
{
	return x != y; //不关心x与y的顺序,只判断是否相同
}

static unsigned int  hashcode(const void *key)
{
	return  (unsigned int)key>>2;  //key地址右移
}

/*表元素节点*/
typedef	struct binding {
		struct binding *link;
		const void *key;  
		void *value;
} *pbinding;

typedef struct table_t {
	int size;      //表身(*list)的长度
	comparator cmp;  
	hashfuc hash;
	unsigned int length;  //存储在表中的元素个数
	unsigned int snapshot; //某一时刻快照标记，用于判断table中的key-value是否改变
	struct binding **list;
} *ptable_t;


struct table_t *table_new(int sz, comparator cp, hashfuc hs)
{
	struct table_t *table;
	int i;
	static int prime[] = {509, 509, 1021, 2053, 4093,
		8191, 16381, 32771, 65521, INT_MAX};
	
	assert(sz >= 0);
	for(i = 1; prime[i] < sz; i++)
		; //找到prime[]提供的最接近sz的数字
	
	table = (struct table_t*) calloc(1, sizeof(*table) +\
		             (sizeof(table->list[0]) * prime[i-1]));
	assert(table);

	table->size = prime[i-1];  //表身长度
	table->cmp = cp ? cp : cmpdef; 
	table->hash = hs ? hs : hashcode;
	table->list = (struct binding **) (table +1); //特别注意内存偏移量
	for(i = 0; i < table->size; i++)
		table->list[i] = NULL;

	table->length = 0;
	table->snapshot = 0;
	return table;
} 
/**给table追加一个新的key-value对，并返回key原始value, 如果不存在key
*    则返回 NULL**/
void *table_put(struct table_t *tb, const void *key, void *value)
{
	int i;
	struct binding *tmp;
	void *preval;  //key对应的当前value值
	assert(tb && key);
	i = (*tb->hash)(key) % tb->size;
	for (tmp = tb->list[i] ; tmp ; tmp = tmp->link) {
		if ((*tb->cmp)(key, tmp->key) == 0)	
			break;
	}/*for*/
	
	if (!tmp) { //不存在该关键字key
		tmp = (struct binding*)calloc(1, sizeof(*tmp));
		assert(tmp);
		tmp->key = key;

		tmp->link = tb->list[i]; //链表头插法，将tmp插入到链表中
		tb->list[i] = tmp;
		tb->length ++;
		preval = NULL;
	}else {
		preval = tmp->value;
	}
	tmp->value = value;
	tb->snapshot = 0; //table初始化时状态
	return preval;
}

/*在当前table中搜索key，若存在返回 key对应的value, 如果不存在返回NULL*/
void *table_get(struct table_t *tb, const void *key)
{
	int i;
	struct binding *p;
	assert(tb && key);

	i = (*tb->hash)(key) % tb->size;
	for(p = tb->list[i]; p; p = p->link) 
		if ((*tb->cmp)(p->key, key) == 0) 
			break;
	
	return p ? p->value : NULL;
}

/*返回table中的元素个数*/
unsigned int table_length(struct table_t *tb)
{
	assert(tb);
	return tb->length;
}

/*删除key所属的binding实例，并没有释放key和value，如果不存在key则返回NULL
 * 否则返回 key所对应的value*/
void *table_remove(struct table_t *tb, const void *key)
{
	int i;
	struct binding **pp; //双重指针,从链表中删除key-value
	struct binding *p; //记录找到的key-value对
	void *value;

	assert(tb);
	tb->snapshot ++;  //表中元素被删除时，更新快照标记
	i = (*tb->hash)(key) % tb->size;
	for(pp = &tb->list[i]; *pp; pp = &(*pp)->link) 
		if ((*tb->cmp)(key, (*pp)->key) == 0) {
			p = *pp;    //p指向找到的key-value
			value = p->value; 
			*pp = p->link; //改变*pp的指向，以便从链表中删除p
			free(p);  
			p = NULL;
			tb->length --;  //表中的元素减一
			return value;
		}/*if*/

	return NULL;
}


/*客户端接口，用于对table的整个元素做相应的处理，
 * cp 处理函数的指针*/
void table_map(struct table_t *tb,\
			   void apply(const void *key, void **val, void *cp),\
			   void *cp)
{
	int i;
	unsigned int snap; //保存当前table表的快照
	struct binding *p;
	snap = tb->snapshot; //记录当前table的状态
	assert(tb && apply);
	for (i = 0; i < tb->size; i++) 
		for (p = tb->list[i]; p; p = p->link) {
			apply(p->key, &p->value, cp);
			assert(snap == tb->snapshot); //保证操作期间table数据一致性
		}/*inside for */
}

/*table的元素转化成一维数组，元素的key和value依次按照顺序存储
  key 存储在数组偶数下标，value存储在奇数下标*/
void **table_to_array(struct table_t *tb, void *end)
{
	int i, j;
	void **ar;
	struct binding *p;

	ar =(void **)calloc((2 * tb->length) + 1, sizeof(*ar));
	assert(ar);
	j = i = 0;
	for (i = 0; i < tb->size; i++) 
		for (p = tb->list[i]; p; p = p->link) {
			ar[j++] = (void *)p->key;
			ar[j++] = (void *)p->value;
		}/*inside for*/

	ar[j] = end;
	return ar;
}

/*释放table表结构，元素节点，value，但没有释放key*/
void table_free(struct table_t *tb)
{
	int i;
	struct binding *p, *pnext;
	assert(tb);
	if (tb->length > 0) {
		for (i = 0; i < tb->size; i++) 
			for (p = tb->list[i]; p; p = pnext) {
				pnext = p->link;
				free(p->value);
				free(p);
		}/*inside for*/

	}/*if*/
	free(tb);
}
