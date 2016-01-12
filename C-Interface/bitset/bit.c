#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bit.h"

struct bit_t {
	int length; //位向量的个数
	unsigned char *bytes; //存储bit的字节
	unsigned long *words; //指向bytes的内存区域
};

//每个unsigned long中包含的bit数
#define BPW (8 * sizeof(unsigned long))
//计算了len个bit位所需要的unsigned long数目
#define nwords(len) ((((len) + BPW-1)&(~(BPW-1))) / BPW)
//计算len个bit位可以分割为多少个byte
#define nbytes(len) ((((len) + 8-1) & (~8-1)) / 8)

//p必须是指针,type是类型
#define NEW(p, len, type) do{\
	p = (type)calloc(len, sizeof(*p));\
	assert(p);\	
} while(0) /*no trailing ;*/



/*
* 新建length个比特位的向量集合,
**/
struct bit_t *bit_new(int length) 
{
	bit_t *bit_set;
	assert(length >= 0);
	NEW(bit_set, 1, bit_t*);
	if(length > 0) {
		NEW(bit_set->words, length, unsigned long*);
	}else {
		bit_set->words = NULL;		
	}
	
	bit_set->length = length;
	bit_set->bytes = (unsigned char *)set->words;
	return bit_set;
}

/*释放bit集合*/
void free_bit(struct bit_t **bit_set)
{
	assert(*bit_set && (*bit_set)->words);
	free((*bit_set)->words);
	free(*bit_set);
	*bit_set = NULL;
}


/*返回bit集合比特位的总和,也就是length字段的值*/
int bit_length(struct bit_t *bit_set) 
{
	assert(bit_set);
	return bit_set->length;
}


/*返回bit集合中有效位为1的个数*/
int bit_count(struct bit_t *bit_set) 
{
	int count; //有效位为1的个数
	int n;  //bit集合按字节索引
	unsigned char c; //临时记录bit集合中的byte
	
	//根据半字节bit位组合,返回相应的bit为1的个数
	static char halfbytelen[16] = {
		0, /*0: 0000*/ /*十进制数： 半字节bit位组合*/
		1, /*1: 0001*/
		1, /*2: 0010*/
		2, /*3: 0011*/
		1, /*4: 0100*/
		2, /*5: 0101*/
		2, /*6: 0110*/
		3, /*7: 0111*/
		1, /*8: 1000*/
		2, /*9: 1001*/
		2, /*10:1010*/
		3, /*11:1011*/
		2, /*12:1100*/
		3, /*13:1101*/
		3, /*14:1110*/
		4  /*15:1111*/ };
	
	assert(bit_set);
	count = 0;
	
	/*
	* 按照byte来循环，以此计算每个byte中有效的bit为1的个数 
	*note: 没有按照逐个遍历集合然和测试每个bit位的算法
	*      该算法会计算多余的bit位, 因为初始化时已经置0，因此不会影响
	**/
	for(n = nbytes(bit_set->length); --n >= 0; ) {
		c = bit_set->bytes[n];
		          /*低四位 + 高四位*/
		count += halfbytelen[c&0xF]	 + halfbytelen[c>>4];	
	}
	return count;
}










