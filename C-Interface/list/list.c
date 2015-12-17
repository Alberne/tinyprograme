#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "list.h"

#define NEW(p, n) do{\
	p = calloc(n, sizeof(*(p)));\
	assert(p);\
	}while(0) 
		
/*新构筑一个List类型的变，其中参数为链表的值*/
List list_new(void *x, ...)
{
	
	va_list ap;
	List list;
	List *plist;
	
	list = NULL;
	plist = &list;
	va_start(ap, x);
	
	//链表尾插发，添加新的节点
	for(; x; x = va_arg(ap, void *)) {
		NEW(*plist ,1);
		(*plist)->ptr = x;
		plist = &(*plist)->next;//移动*P,指向链表最后一个节点		
	}
	*plist = NULL; //链表尾节点置NULL
	va_end(ap);
	return list;
}

/*给链表追加一个新值，并生成新的节点，将其插入
*   如果传入的list为空，则生成具有一个节点的新链表*/
List list_push(List list, void *x)
{
	List p;
	NEW(p ,1);
	p->ptr = x;
	p->next = list;
	return p;
}

/*取出链表中的第一个元素*/
List list_pop(List list, void **x)
{
	List p;
	p = NULL;
	if(list) {
		p = list->next; //链表头节点的下一个节点
		if(x) {
			*x = list->ptr;
		}
	free(list); //释放头节点	
	}
	return p;
}

/*复制list的内容到新的链表，并返回*/
List list_copy(List list)
{
	List newlist;
	List *p;
	p = &newlist;
	
	for(; list; list = list->next) {
		NEW(*p ,1);
		(*p)->ptr = list->ptr; //复制
		p = &(*p)->next; //移动*p指针
	}
	*p = NULL;
	return newlist;
}

/*链表逆置*/
List list_reverse(List list)
{
	List newlist; //逆置的新链表
	List next;
	newlist = NULL;
	
	for(; list; list = next) {
		next = list->next; //记录下一个节点
		list->next = newlist; //指针重定向
		newlist = list;  //newlist向前移动
	}
	return newlist;	
}

/*链表合并*/
List list_append(List list, List tail)
{
	List *p;
	p = &list;
	
	while(*p) {
		p = &(*p)->next; /*循环走到list链表的尾节点*/
	}
	*p = tail; //链接tail链表
	return list;
}

/*取得链表长度*/
int  list_length(List list)
{
	int n;
	n = 0;
	for(; list; list = list->next) {
		n++;
	}
	return n;
}
/*释放链表,free_value 为1则释放，链表节点的ptr指针，否则只释放链表节点*/
void list_free(List *list, char free_value)
{
	List next;
	if(list)
		return ;
	for(; *list; *list = next) {
		next = (*list)->next;
		if(free_value) { //释放链表节点ptr的内存
			free((*list)->ptr);
		}
		free(*list);
	}/*for*/
	*list = NULL;
}

/*给链表节点执行自定义的行为，该行为是参数 c 指向的函数*/
void list_map(List list,\
		void apply(void **x, void *c), void *c)
{
	assert(apply);
	for(; list; list = list->next) {
		apply(&list->ptr, c);
	}
}

/*将链表节点的值，组装成一个指针数组*/
void **list_to_array(List list, void *end)
{
	int i, n;
	i = 0;
	n = list_length(list);
	void **ar;
	NEW(ar, (n+1));
	
	for(; i < n; i++) {
		ar[i] = list->ptr;
		list = list->next;
	}
	ar[i] = end;
	return ar;
}
