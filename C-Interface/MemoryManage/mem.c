#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mem.h"

#define NALLOC  4096  /*重新分配的内存大小*/
#define NNODES  512   /*一次内存管理节点的数*/
#define HASH(p, t) (((unsigned long)(p) >> 3) & (sizeof(t)/sizeof((t)[0]) - 1))

static  struct memnode {
	struct memnode *free; //内存空闲时使用
    struct memnode *link; 
    const void *ptr; //内存地址
    long size; //内存大小
    const char *_file_locate; 
    int _file_line;
} *htab[1024];

static struct memnode freelist = {&freelist};
static void *mem_alloc(long nbytes);
static struct memnode *dalloc(void *ptr, long size);

/*查找管理ptr内存地址的节点, 如果未找到则返回为NULL*/
static struct memnode *find(const void *fptr)
{
	struct memnode *tmp;
	unsigned int h;
	tmp = NULL;
	h = 0;
	
	h = HASH(fptr, htab);
	tmp = htab[h];
	//查找 fptr的内存节点,如果为找打
	while(tmp  && tmp->ptr != fptr) {
		tmp = tmp->link;
	}
	return tmp;
}

void *mem_resize(void *ptr, long nbytes) 
{
	struct memnode *newpb;
	void *newptr;
	long cpybytes;  //重新分配内存后，给新内存实际copy的字节数
	newpb = newpb = NULL;
	cpybytes = 0;
	assert(ptr && nbytes>0);
	if((newpb = find(ptr)) == NULL || newpb->free) {
			newpb ? puts("no find memory\n") : puts("this memory always free"); 
			return NULL;
		}/*if*/
	newptr = mem_alloc(nbytes); //重新开辟新的内存空间
	cpybytes = nbytes < newpb->size ?  nbytes : newpb->size;
	memcpy(newptr, ptr, cpybytes); //将原有的内存数据copy到新的内存中
	mem_free(ptr);// 释放旧的内存空间
	return newptr;
}

/* “释放” 内存块*/
void mem_free(void *ptr)
{
	struct memnode *pb;
	pb = NULL;
	//有效地址才需要释放
	if(ptr) {
		
		/*如果为找到ptr所属的内存节点， 
		  *或者pb已经加入到空闲表中, 释放失败*/
		if((pb = find(ptr)) == NULL || pb->free) {
			pb ? puts("no find memory\n") : puts("this memory always free"); 
			return ;
		}/*if*/
		
		//将管理ptr的内存节点加入到空闲链表中
		pb->free = freelist.free;
		freelist.free = pb;
	}/*if*/

}

void *mem_calloc(long count, long nbytes)
{
	void *ptr;
	assert(count>0 && nbytes>0);	
	ptr = mem_alloc(count * nbytes);
	memset(ptr, '\0', count*nbytes);
	return ptr;
}


/**
* @parm: nbytes   内存字节数
* return 内存指针
**/
static void *mem_alloc(long nbytes)
{
	struct memnode *bp;
	void *ptr;
	struct memnode *newptr; //空闲内存不够时，重新malloc
	unsigned int h; //哈希值
	bp = NULL;
	ptr = NULL;
	assert(nbytes > 0); //
	
	//使用“first match”算法在空闲的内存块链表中找到一个空闲块
	for(bp = freelist.free; bp; bp = bp->free) {
		
		/*当前的空闲块的内存节点大小满足需求*
		* 如果不满足继续for循环查找，直到遍历完整链表
		*    如果未找到，则需要重新malloc分配节点，加入freelist中
		*        再次进入下次循环*/
		if(bp->size > nbytes) {
			bp->size -= nbytes; 
			ptr = (char *)bp->ptr + bp->size; //移动指针
			//将memnode 加入哈希表(htab)中
			if((bp = dalloc(ptr, nbytes)) != NULL) {
				h = HASH(ptr, htab);
				bp->link = htab[h];
				htab[h] = bp;
				return ptr;  //内存分配成功，并返回
			}else {
				assert(0);  //失败
			}
		}/*if*/
		
		/*循环完整个空闲块,未找到空闲块，此时重新分配内存*/
		if(bp == &freelist) {
			//ptr 为实际内存地址
			if((ptr = malloc(nbytes + NALLOC)) == NULL\
					|| ((newptr = dalloc(ptr, nbytes + NALLOC)) == NULL)) {			
					
						assert(!"malloc  failed\n");
						
					}/*if*/
			//将新分配的节点,头插法加入到freelist链表中
			newptr->free = freelist.free;
			freelist.free = newptr;
			
		}/*if*/
		
	}/*for*/
	assert(!"no happend\n");
	return NULL;	
}



/*内存节点(struct memnode)分配器, 并给节点分配实际内存地址, 内部用*/

static struct memnode *dalloc(void *ptr, long size)
{
	static struct memnode *availnode = NULL;
	struct memnode *ret = NULL;
	static int nleft = 0;
	
	if(nleft <= 0) {
		//一次分配512个内存节点
		availnode = malloc(NNODES * sizeof(*availnode));
		if(availnode == NULL) {
			return NULL;
		}
		nleft = NNODES;
	}/*if*/
	
	availnode->ptr = ptr;
	availnode->size = size;
	availnode->free = NULL;
	availnode->link = NULL;
	
	nleft--;
	ret = availnode;
	availnode++; //指向下一个节点
	
	return ret; 
	
}
