#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../C-Interface/set/set.h"


#define array_len(ar) (sizeof(ar) / sizeof(ar[0]))  //ar必须是数组 

/*内存分配器管理
* p：必须是指针，分配完后指向内存地址
* c：内存块的个数
* type:p的指针类型
**/
#define PNEW(p,c,type) do{\
		p = (type)calloc(c, sizeof(*(p)));\
		assert(p);\
}while(0) /*no trailing ;*/

typedef struct product_detail{
	char bar_code[15];     // 商品条形码，也是商品ID 
	char name[15];     
	float unit_price;     //单价
	char  unit[8]; 
	char category[15];     //商品类别 
}*product_detail;

typedef struct sales_promotion{
	char priority;  //如果优惠活动冲突时，优先使用哪个优惠活动 
	struct promotion_one_info{
		char promotion_type; //促销活动，默认是0x00，不参加任何活动 
		char statement[50];   //促销活动名 
		struct set_t *sales_set; //促销活动的商品 
		struct promotion_one_info *next_promotion; //下一个优惠活动 
	}*promotion_all_info;
}*sales_promotion;

typedef struct sales_products{
	char bar_code[15];  //商品条形码 
	unsigned int sum;   //商品数量 
}*sales_products;


//通过比较商品的条形码，判断是否为同一商品 
static int compare_product(const void *x, const void *y)
{
    product_detail product1, product2;
    product1 = (struct product_detail *)x;
    product2 = (struct product_detail *)y; 
       
	return strcmp((const char *)product1->bar_code, (const char *)product2->bar_code);
} 

static unsigned int hashcode(const void *member) 
{
	unsigned int h;
	product_detail key;
	int len;
    int i;
	const char *val;
	key = (struct product_detail *)member;
	val = (char *)key->bar_code;
	len = strlen((const char *)key->bar_code);
	h = 0;
	for(i = 0; i < len; i++) 
		h = 31 * h + val[i];
	return h;
}

//utility to jump whitespace and cr/1f

static const char *skip(const char *str)
{
	while (str && *str && (unsigned char)*str <= 32)
		str++; 
	
	return str;
}


static struct set_t *get_products(void);
void get_promotion();//设置优惠活动  
void prase_JSON_array(const char *val); //解析json格式中的数组，


int main(int argc, char *argv[]) {
	
	//从收银系统导入商品库存数据 
	struct set_t *products; //库存商品集合 
	struct sales_promotion product_promotion; //优惠活动信息
	products = get_products();
	 
	//从收银系统取得优惠活动信息
	product_promotion = get_promotion();
    
     
	 
	




	return 0;
}


/*根据传入的JSON数据,解析出商品条形码,数量,然后填充到集合中
*@parm: value JSON数据
*@parm:c1 集合结构
*@parm: apply 业务函数
**/
void prase_JSON_array(const char *value, void *c1,\
							void apply(int, char*, void *)) 
{
	char *p;
	int sum;      //商品数量
	char *item; //商品条形码

	n = 0;
	value = skip(value); //跳过开头无用的字符	
	if (*value!='[') {
		return;
	}	/* not an array! */
	
	value = skip(value); 
	if (*value==']')
		return;	/* empty array*/
	
	/*解析JSON, 将数据填充到数据结构中*/
	do {
		value = parse_value(value, &sum, &item);
		apply(sum, item, c1);          //将商品信息,填充到集合中
	} while(*(value = skip(value)) == ',');
	
	value = skip(value);
	asser(*value == ']'); //解析完毕,确保JSON格式正确
}

/*解析 'ITEMxxxxx-n', 'ITEMxxxxx'字符串
* 'ITEMxxxxx-n' 解析成 'ITEMxxxxx' 和 n 
* 'ITEMxxxxx'   解析成 'ITEMxxxxx' 和 0*/
static char *parse_value(char *item, int *n, char **value) 
{
	char c = '-';
	char *pitem, p;
	char *val;  //存放'ITEMxxxxxx'字符串
	int number; //取得'ITEMxxxxx-n'中的'n'
	
	number = 0;
	pitem = item;
	skip(item);
	assert(*pitem == '\'');
	pitem++;

	p = strchr(item, c); //检测 '-'字符
	if(!p) {  //‘ITEMxxxxx’格式
		p = strchr(pitem, '\'');
		PNEW(val, (p-pitem) + 2, char*);
		strncpy(val, pitem, (p-pitem)+1);
		
		*n = 0;
		*value = val;
	}else {  // ‘ITEMxxxxx-n’格式
		
		PNEW(val, (p-pitem) + 2, char*);
		strncpy(val, pitem, (p-pitem)+1);
		
		p++; //跳过'-'字符
		while(*p >= '0' && *p <= '9') 
			number = (number * 10) + (*p++ - '0');
		
		*n = number;
		*value = val;
	}/*else*/
	
	return p;
}


//设置优惠活动 
struct sales_promotion *get_promotion() 
{
    static struct sales_promotion *promotions;
    struct promotion_one_info *p; //临时变量  
    struct set_t *pset; //临时变量
	int i;  
    static struct sales_products discount_promotion[] = { //折扣商品信息
		{"ITEM00001", 0},
		{"ITEM00002", 0},
		{"ITEM00003", 0},
		{"ITEM00004", 0}
	};
	static struct sales_products one_free_promotion[] = { //买二送一商品信息
		{"ITEM00001", 0},
		{"ITEM00002", 0},
		{"ITEM00003", 0}
	};
	 
    PNEW(promotions, 1, struct sales_promotion *);
	promotions->promotion_all_info = NULL;
	
    puts("--------通过收银系统设置优惠活动--------\n");
    /*假设当前有以下两种活动*
    *  1. 买二送一活动
    *  2. 9.5折优惠 */
    
     promotions->priority = 1;  //当某种商品满足两种优惠时，优先选则活动一
     
     /*首先设置优惠活动1信息*/
     PNEW(promotions->promotion_all_info, 1,\
                        struct promotion_one_info *);
     p = promotions->promotion_all_info;
	 p->next_promotion = NULL;
     pset = p->sales_set; 
     p->promotion_type = 1;
     strcpy(p->statement, "买二送一活动");
     pset = set_new(100, NULL, NULL);
	 for(i = 0; i < array_len(one_free_promotion); i++) {
		 set_put(pset, &one_free_promotion[i]);		 		 
	 }
 
     /*其次设置优惠活动2信息*/ 
	 PNEW(p, 1, struct promotion_one_info *); //分配活动2商品所需的内存
	 //链表头插法
	 p->next_promotion = promotions->promotion_all_info;
     promotions->promotion_all_info = p;
	 pset = p->sales_set;
	 p->promotion_type = 2;
	 strcpy(p->statement, "9.5折优惠");
	 pset = set_new(100, NULL, NULL);
	 for(i = 0; i < array_len(discount_promotion); i++) {
		 set_put(pset, &discount_promotion[i]);
	 }
	 
	 return promotions;
}

/*从收银系统取得商品信息*/ 
static struct set_t *get_products(void)
{
    static struct set_t *product_set;
    int i;
	static struct product_detail products[] = {
		{"ITEM00001", "橘子", 10, "斤", "水果"},
		{"ITEM00002", "苹果", 100, "斤", "水果"},
		{"ITEM00003", "香蕉", 3.5, "斤", "水果"},
		{"ITEM00004", "柚子", 10.5, "斤", "水果"},
		{"ITEM00005", "瓜子", 99, "斤", "干果"},
		{"ITEM00006", "麻子", 19.5, "斤", "干果"},
		{"ITEM00007", "开心果", 10.5, "斤", "干果"},
		{"ITEM00008", "花生", 10.5, "斤", "干果"}
	};
	
	/*初始化商品信息集合*/
	product_set = set_new(1024, compare_product, hashcode);
    for(i = 0; i < array_len(products); i++) {
        set_put(product_set, &products[i]);
    }
    puts("商品信息加载完毕\n");
    return product_set;
}


