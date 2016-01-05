#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef int (*comparator)(const void *x, const void *y); /*定义一个比较器类型*/
typedef unsigned int (*hashfuc)(const void *key);/*定义一个hash函数类型*/


//该比较器,只能判断x & y是否为同一块内存区
static int cmpdef(const void *x, const void *y)
{
	return x != y; //不关心x与y的顺序,只判断是否相同
}

static unsigned int  hashcode(const void *key)
{
	unsigned int h;
	int len;
        int i;
	const char *val;
	val = (char *)key;
	len = strlen((const char *)key);
	h = 0;
	
	for(i = 0; i < len; i++) 
		h = 31 * h + val[i];

	return h;
}

typedef struct member_set;
struct set_t {
	int length; //集合的大小
	int size;  //集合索引的大小
	unsigned int snapshot;  //集合快照，防止集合被篡改
	comparator cmp;
	hashfuc hash;
	
	struct member_set { //集合元素
		struct member_set *next;
		const void *member; 
	} **buckets; //数据结构形状类似于哈希表
};

/*新建一个集合表,该集合没有元素，只有集合索引list*/
struct set_t *set_new(int sz, comparator cp, hashfuc hs)
{
	struct set_t set;
	int i;
	
	static int prime[] = {509, 509, 1021, 2053, 4093,
		8191, 16381, 32771, 65521, INT_MAX};
	
	assert(sz >= 0);
	for(i = 1; prime[i] < sz; i++)
			; //找到prime[]提供的最接近sz的数字
	
	//初始化一个只有集合索引list，并没有集合元素,集合list的大小为 prime[i-1]
	set = (struct set_t *) calloc(1, sizeof(*set) +\
			(sizeof(set->buckets[0]) * prime[i-1]));		
	assert(set);
	
	set->size = prime[i-1];
	set->length = 0;
	set->snapshot = 0; //set集合初始化状态
	set->cmp = cp ? cp : cmpdef;
	set->hash = hs ? hs : hashcode;
	/*特别注意内存偏移量，因为calloc分配时，就增加了sizeof(*set)*/
	set->buckets = (struct member_set **) (set + 1);
	
	for(i = 0; i < set->size; i++) 
		set->buckets[i] = NULL;
	
	return set;
}

/*检索, member是否在集合set中，若存在返回非零值，否则返回零*/
int set_member(struct set_t *set, const void *member) 
{
	int i;
	struct member_set *p;
	
	assert(set);
	assert(member);
	
	i = (*set->hash) (member) % set->size; //哈希值
	
	for(p = set->buckets[i]; p; p = p->next) 
		if((*set->cmp)(member, p->member) == 0)
			break;  //member存在集合set中
	
	return p != NULL;
}


/*向集合中添加一个新的元素member,如果集合中已经存在，则不添加*/
void set_put(struct set_t *set, const void *member)
{
	int i;
	struct member_set *p;
	assert(set);
	assert(member);
	
	i = (*set->hash)(member) % set->size;
	
	for(p = set->buckets[i]; p; p = p->next) 
		if((*set->cmp)(member, p->member) == 0)
			break;  //member存在集合set中
	
	//set中不存在member元素
	if(p == NULL) {
		p = calloc(1, sizeof(*p));
		assert(p);
		p->member = member;
		
		//链表头插法,将p加入到set中
		p->next = set->buckets[i]; 
       	        set->buckets[i] = p;
		set->length++;
		set->snapshot++; //set修改记录
	}
}

/*如果集合中存在member则删除，删除成功则返回1*/
int set_remove(struct set_t *set, const void *member) 
{
	int i;
	struct member_set **pp;
	struct member_set *p;
	
	assert(set && member);
	set->snapshot++;  //set集合被修改，快照记录
	i = (*set->hash)(member) % set->size;
	
	for(pp = &set->buckets[i]; *pp; pp = &(*pp)->next) 
		if((*set->cmp)(member, (*pp)->member) == 0) {
			p = *pp;  //p 指向找到的节点
			*pp = p->next;// pp是二级指针,覆盖点前节点的地址
			free(p);
			set->length--;
			break;
		}/*if*/
	
	return *pp != NULL;
}

//返回集合的大小
int set_length(struct set_t *set) 
{
	assert(set);
	return set->length;
}

//释放set集合
void set_free(struct set_t **pset) 
{
	int i;
	struct member_set *p, *q;
	
	assert(pset && *pset);
	
	//集合存在元素,释放集合中的元素
	if((*set->length) > 0) {
		//分别释放每个buckets中的元素
		for(i = 0; i < (*pset)->size; i++) 
			for(p = (*pset)->buckets[i]; p; ) {
				q = p->next;
				free(p);
				p = q
			}/*for*/
		
	}/*if*/
	
	free(*pset);
	*pset = NULL;
}

//给集合中每个元素实施一个
void set_map(struct set_t *set,\
				void apply(const void *member, void *c),\
				void *c) 
{
	unsigned int stmp;
	int i;
	struct member_set *p;
	assert(set);
	assert(apply);
	
	stmp = set->snapshot;
	
	for(i = 0; i < set->size; i++) 
		for(p = set->buckets[i]; p = p->next) {
			apply(p->member, c);
			assert(set->snapshot == stmp); //保证,set集合在此操作时没有被外部程篡改
		}/*for*/
}

/*将集合中的数据压缩成一维数组并返回
* @parm: end是数组最后一个元素*/
void **set_toarray(struct set_t *set, const void *end) 
{
	int i, j;
	void **ar;
	struct member_set *p;
	
	assert(set);
	j = 0;
	ar = calloc((set->length + 1) * sizeof(*ar));
	//将集合中所有的元素存储在数组ar中
	for(i = 0; i < set->size; i++) 
		for(p = set->buckets[i]; p; p = p->next) {
			ar[j++] = (void *)p->member;
		}/*for*/
		
	ar[j] = end;
	return ar;
}

/*将集合t复制到一个新的集合set中并返回,
*@parm sz:不是集合元素的大小，而是集合结构中buckets(也就是t->size)的大小
*/
static struct set_t *copy(struct set_t *t, int sz) 
{
	struct set_t *set;
	int i;
	int h;
	struct member_set *p;
	struct member_set *pnew;
	
	assert(t);
	set = set_new(sz, t->cmp, t->hash);
	
	//集合t中的元素复制到set集合中
	for(i = 0; i < t->size; i++) {
		for(p = t->buckets[i]; p; p = p->next) {
			h = (*set->hash)(p->member) % set->size; 
			pnew = (struct member_set *)calloc(1, sizeof(*pnew));
			assert(pnew);
			pnew->member = p->member;
			//链表头插法
			pnew->next = set->buckets[i];
			set->buckets[i] = pnew;
			set->length++;
			
		}/*for*/
		
	}/*for*/
	
	return set;
}

 /*     +-------------------------------+
        |   下面是几种常见的集合运算    |
	+-------------------------------+
	| 并集(s+t) : set_union         |
	| 交集(s*t) : set_inter         |
	| 差集(s-t) : set_minus         |
	| 称差集(s/t): set_diff         |
	+-------------------------------+         */


/*求集合t与集合s的并集(t + NULL = t; NULL + s = s)
*  如果t和s都为NULL,则抛出运行时异常
*/
struct set_t *set_union(struct set_t *t, struct set_t *s) 
{
	struct member_set *p;
	struct set_t *set;
	int arith_max;  //s与t中最大的buckets数
	int i;
	
	if(t == NULL) { //NULL + s = s
		assert(s);
		set = copy(s, s->size);
	}else if(s == NULL) { //t + NULL = t
		set = copy(t, t->size);
	}else {  // t + s
		//保证两个集合的比较算法,和哈希算法相同(指向同一个函数)
		assert(s->cmp == t->cmp && s->hash == t->hash);
		arith_max = (t->size > s->size) ? t->size : s->size;
		set = copy(s, arith_max);//将s复制到新的集合ret中
		
		for(i = 0; i < t->size; i++)
			for(p = t->buckets[i]; p; p = p->next) 
				set_put(set, p->member); //将t中的元素复制到ret中
	}/*else*/
	
	return set;
}

/*求集合s与集合t的交集(s*NULL = NULL; NULL*t = NULL)
* (s*t :s与t中共有的元素) */
struct set_t *set_inter(struct set_t *s, struct set_t *t) 
{
	struct set_t *set;
	struct member_set *q, *pnode;
	const void *member;
	int i;
	int h;
	int arith_min; //s与t中最小的buckets数
	
	if(s == NULL) { //t*NULL = NULL
		assert(t);
		set = set_new(t->size, t->cmp, t->hash);
	}else if(t == NULL) { //s*NULL = NULL
		set = set_new(s->size, s->cmp, s->hash);
	}else if(s->length > t->length) {
		set = set_inter(t, s);
	}else {
		//保证两个集合的比较算法,和哈希算法相同(指向同一个函数)
		assert(s->cmp == t->cmp && s->hash == t->hash);
		arith_min = (s->size > t->size) ? t->size : s->size;
		//set 是新建的空集合,buckets取两(s,t)者最小
		set = set_new(arith_min, s->cmp, s->hash);
		
		//循环遍历较大的集合,查找两个集合中共有的元素
		for(i = 0; i < t->size; i++) 
			for(q = t->buckets[i]; q; q = q->next) {
				if(set_member(s, q->member)) { //集合s中是否存在该元素
					member = q->member;
					h = (*set->hash)(member) % set->size;
					pnode = (struct set_member*)calloc(1, *pnode);
					assert(pnode);
					pnode->member = member;
					
					pnode->next = set->buckets[i]; //链表头插法
					set->buckets[i] = pnode;
					set->length++;
				}/*if*/
			}/*for*/
		
	}/*else*/
	return set;	
}




