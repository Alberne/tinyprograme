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

//低byte,置位时所需的掩码
unsigned char low_byte_mask[] = {
	0xFF,    /*1111 1111*/
	0xFE,    /*1111 1110*/
	0xFC,    /*1111 1100*/
  0xF8,    /*1111 1000*/
  0xF0,    /*1111 0000*/
  0xE0,    /*1110 0000*/
  0xC0,    /*1100 0000*/
  0x80    /*1000 0000*/
	};

//高byte, 置位时所需的掩码
unsigned char high_byte_mask[] = {
	0x01,     /*0000 0001*/
	0x03,     /*0000 0011*/
	0x07,     /*0000 0111*/
	0x0F,     /*0000 1111*/
	0x1F,     /*0001 1111*/
	0x3F,     /*0011 1111*/
	0x7F,     /*0111 1111*/
	0xFF      /*1111 1111*/
	};

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
		NEW(bit_set->words, nwords(length), unsigned long*);
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

/*取得第n个比特位的值*/
int bit_get(struct bit_t *bit_set, int n)
{
	int byte_val = 0;
	assert(bit_set);
	assert(n >= 0 && n < bit_set->length);
	/*n的bit位置在第(n/8)个字节中,第(n%8)的位置，但是bit位是右向左排列*/
	byte_val = (bit_set->bytes[n/8]>>(n%8)) & 0x1;//最右一位bit
	return byte_val;
}



/*设置第n位bit位的值,并返回原始的值
*@parm: bit 需要设置的值
*@parm: n 第几个bit位*/
int bit_put(struct bit_t *bit_set, int n, int bit) 
{
	int previous_value;
	assert(bit_set);
	assert(bit == 0 || bit == 1);
	assert(n >= 0 && n < bit_set->length);

	previous_value = bit_set->bytes[n/8]>>(n%8); //取得原始的值
	if(bit == 1) {
		bit_set->bytes[n/8] |= (0x1 << (n%8)); //制造一个掩码 n%8 为1, 其它bit值为0,然后,按位或
	}else {
	 	bit_set->bytes[n/8] &= (0x1 << (n%8));  //制造一个掩码 n%8 为0, 其它bit值为1,然后,按位与
	}

	return previous_value;
}


/*指定一个范围,将其中bit值置为1
*@parm: low 低位
*@parm: high 高位*/

void bit_set(struct bit_t *bit_set, int low, int high)
{
	int i;  //用于遍历指定范围中的每个byte
	int low_byte;  //低bit位所在的byte
	int high_byte; //高bit位所在的byte
	
	low_byte = low / 8;
	high_byte = high / 8;
	assert(bit_set);
	assert(0 <= low && high < bit_set->length);
	assert(low <= high);
	
	if(low_byte < high_byte) {  //跨字节,指定范围不在同一个byte中
		
		bit_set->bytes[low_byte] |= low_byte_mask[low%8];    //低byte置位
		for(i = low_byte+1; i < high_byte; i++) {
				bit_set->bytes[i] = 0xFF;       //中间的byte全置位
		}
		bit_set->bytes[high_byte] |= high_byte_mask[high%8]; //高byte  
		
	}else {  //指定范围在同一个byte中
		bit_set->bytes[low_byte] |= (low_byte_mask[low%8]) & (high_byte_mask[high%8]);
	}	
	
} 


/*指定一个范围，将其中的bit位清零
* @parm: low 地位
* @parm: hign 高位*/
	
void bit_clear(struct bit_t *bit_set, int low, int high)
{
	int i;  //用于遍历指定范围中的每个byte
	int low_byte;  //低bit位所在的byte
	int high_byte; //高bit位所在的byte
	
	low_byte = low / 8;
	high_byte = high / 8;
	assert(bit_set);
	assert(0 <= low && high < bit_set->length);
	assert(low <= high);
	
	if(low_byte < high_byte) {  //跨字节,指定范围不在同一个byte中
		
		bit_set->bytes[low_byte] &= ~low_byte_mask[low%8];    //低byte置位
		for(i = low_byte+1; i < high_byte; i++) {
				bit_set->bytes[i] = 0x00;       //中间的byte全置位
		}
		bit_set->bytes[high_byte] &= ~high_byte_mask[high%8]; //高byte  
		
	}else {  //指定范围在同一个byte中
		bit_set->bytes[low_byte] &= ~(low_byte_mask[low%8]) & (high_byte_mask[high%8]);
	}
}


