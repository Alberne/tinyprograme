#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "set.h"
#include "tw.h"

//通过比较商品的条形码，判断是否为同一商品
static int compare_product(const void *x, const void *y)
{
    const char *product1;
    const char *product2;
    product1 = (const char *)x;
    product2 = (const char *)y; 
    return memcmp(product1, product2, bar_code_len);
} 

/*计算条形码的哈希值,该函数只适用于member变量中前15个字节存储的是条形码*/
static unsigned int hashcode(const void *member) 
{
	unsigned int h;
        int i;
	const char *val;
	val = (const char *)member;
	h = 0;
	for(i = 0; i < bar_code_len; i++) 
		h = 31 * h + val[i];

	return h;
}

/*---------------收银系统模块----------*/
//设置优惠活动 
struct sales_promotion *get_promotion() 
{
	static struct sales_promotion *promotions;
        struct promotion_one_info *p; //临时变量  
    	struct set_t *pset; //临时变量
    	int i;  
    	static struct sales_products discount_promotion[] = { //折扣商品信息
		{"ITEM00001", 1},
		{"ITEM00002", 1}
		//{"ITEM00003", 0},
		//{"ITEM00004", 0}
	};
	static struct sales_products one_free_promotion[] = { //买二送一商品信息
		{"ITEM00001", 1},
		{"ITEM00003", 1}
		//{"ITEM00003", 0}
	};
	 
    	PNEW(promotions, 1, struct sales_promotion *);
	promotions->promotion_all_info = NULL;
	
    	puts("--------通过收银系统设置优惠活动--------\n");
    /*假设当前有以下两种活动*
    *  1. 买二送一活动
    *  2. 9.5折优惠 */
    	promotions->priority = DISCOUNT;  //当某种商品满足两种优惠时，优先选则活动一
     
     /*首先设置优惠活动1信息*/
     	PNEW(p, 1, struct promotion_one_info *);
     	p->next_promotion = promotions->promotion_all_info;
     	promotions->promotion_all_info = p;
     	p->type = FREE_ONE;
     	strcpy(p->statement, "买二送一活动");
     	pset = set_new(100, compare_product, hashcode);
     	p->sales_set = pset; 

     	for(i = 0; i < array_len(one_free_promotion); i++) {
     		set_put(pset, &one_free_promotion[i]);		 		 
     	}
 
     	/*其次设置优惠活动2信息*/ 
	 PNEW(p, 1, struct promotion_one_info *); //分配活动2商品所需的内存
	 //链表头插法
	 p->next_promotion = promotions->promotion_all_info;
	 promotions->promotion_all_info = p;
	 p->type = DISCOUNT;
	 strcpy(p->statement, "9.5折优惠");
	 pset = set_new(100, compare_product, hashcode);
	 p->sales_set = pset;

	 for(i = 0; i < array_len(discount_promotion); i++) {
		 set_put(pset, &discount_promotion[i]);
	 }
	 
	 return promotions;
}

/*从收银系统取得商品信息*/ 
static struct set_t *get_products()
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
	return product_set;
}
/*------------收银系统--end------------*/


int main(int argc, char *argv[]) {
	
	//从收银系统导入商品库存数据 
	struct set_t *products; //库存商品集合 
	struct sales_promotion *product_promotion; //优惠活动信息
	char shopping[] = "[\
				'ITEM00001',\
				'ITEM00002-2',\
				'ITEM00003-3',\
				'ITEM00004-4'\
			]"; /*购物车商品*/
	
	
	struct set_t *shopping_set; //购物车商品集合
	
	shopping_set = set_new(100, compare_product, hashcode);
	//取得库存商品,打印小票时需要
	products = get_products();
	
	//取得优惠活动信息,结算时需要
	product_promotion = get_promotion();
    
	//取得购物车商品集合,结算时需要
    	prase_JSON_array(shopping, shopping_set, pack_cart); 
	 
	//结算
	pay(products, product_promotion, shopping_set);
	
	return 0;
}

void pay(struct set_t *store, struct sales_promotion *promotions, struct set_t *shopping)
{
	struct set_t *pomo_set;  //购物车中的促销商品
	struct set_t* double_pomo_set;  //购物车中同时享受两种优惠的商品
	struct set_t *temp1, *temp2;  //临时变量
	struct set_t *pure_goods;  //购物车中不享受优惠的商品
	struct promotion_one_info *pomo_info; //促销活动
	enum promotion_category pomo_priority; //如果商品享受两种优惠，应该优先使用哪种
	float cost_save;  //优惠节约金额
	float expenditure; //消费总计
	struct free_one_list *free_list; //赠送的商品

	cost_save = expenditure = 0; 
	pomo_info = promotions->promotion_all_info;//指向优惠活动的头指针
	pomo_priority = promotions->priority;
	free_list = NULL;
	
	double_pomo_set = shopping;
	pure_goods = shopping;
	while(pomo_info) {  //循环遍历每种活动
		//先计算享受两种优惠的商品
		temp1 = double_pomo_set; 
		double_pomo_set	= set_inter(double_pomo_set, pomo_info->sales_set);
		
		if (temp1 != shopping) {
			set_free(&temp1);
		}/*释放set_inter生成的临时的set*/

		//计算没有优惠的商品
		temp2 = pure_goods;
		pure_goods = set_minus(pure_goods, pomo_info->sales_set);
		if(temp2 != shopping) {
			set_free(&temp2);
		}/*释放set_minus生成的临时的set*/

		pomo_info = pomo_info->next_promotion;
	}/*while*/
	
	//打印小票头信息
	print_title();
	//计算没有优惠
	settle_product(store, pure_goods, &cost_save, &expenditure);
	//计算二重优惠
	settle_double_promotion(store, pomo_priority, double_pomo_set, &cost_save, &expenditure);

	//计算一种优惠的商品,并结算
	for (pomo_info = promotions->promotion_all_info;\
		pomo_info; pomo_info = pomo_info->next_promotion){
		           
			switch (pomo_info->type) {
					
			case DISCOUNT: //95折扣
				temp1 = set_inter(pomo_info->sales_set,shopping);
				settle_discount(store, temp1, double_pomo_set, &cost_save, &expenditure);
				if(temp1)
					set_free(&temp1);
				 break;

			case FREE_ONE: //买二送一
                                temp1 = set_inter(pomo_info->sales_set,shopping);
				settle_free_one(store, temp1, double_pomo_set, &cost_save, &expenditure);
				if(temp1)
					set_free(&temp1);
				break;
			default:
				assert(!"no this promotion\n");
				break;
			}/*switch*/
	}/*for*/
	
	//赠送商品列表打印
	free_list = put_free_one_list(NULL, 0);
	free_one_print(free_list);
	//小票末尾信息打印
	print_tail(cost_save, expenditure);

}

void print_title()
{
	printf("***<没钱赚商店>购物清单***\n");
	
}
void print_tail(float saveing, float consume)
{
	printf("总计:%.1f(元)\n", consume);
	if(saveing) {
		printf("节省:%.1f(元)\n",saveing);
	}
	printf("******************************");
}
void free_one_print(free_one_list list)
{
	free_one_list p;
	if (!list)
		return;

	printf("\n--------------------\n");
	printf("买二赠一商品\n");

	for( ;list; ) {
		p = list;
		printf("名称:%s,",p->name);
		printf("数量:%u\n", p->count);
		list = p->next;
		free(p);
	}
	printf("\n--------------------\n");
}

/*没有优惠的商品结算
*@parm: store 库存商品
*@parm: products 购物车中不享受优惠的商品
*@parm: saveing 节省总计
*@parm: consume 消费总计*/
void settle_product(struct set_t *store, struct set_t *products,\
			 float *saveing, float *consume)
{
	if (products) {
		set_map(products, settle_product_print, (void *)store, consume, saveing);
	}	
}
/* 不享受优惠活动的商品计算，并打印小票
*@parm: member 商品条形码信息
*@parm: c 库存商品集合
*@parm: c1 总计金额
*@parm: c2 节省金额
*@parm: c3 赠送商品列表*/
void settle_product_print(const void *member, void *c, float *c1, float *c2)
{
	struct product_detail *product;
	struct sales_products *sales;
	unsigned int count;  //购买的商品数量
	float money; //小记金额
	float price; //单价

	price = count = money = 0;
	sales = (struct sales_products *)member;
	product = (struct product_detail *)set_member((struct set_t *)c, sales);
	assert(product && sales);
	count = sales->sum;
	price = product->unit_price;
	money = price * count;
	printf("名称:%s,",product->name);
	printf("数量:%d,",count);
	printf("单价:%.1f(元),",price);
	printf("小记:%.1f(元)\n", money);

	(*c1) += money; //更新消费总计
}

/*多重优惠商品结算*/
void settle_double_promotion(struct set_t *store, enum promotion_category type,
					struct set_t *products, float *saveing,\
								float *consume)
{
	if(type == FREE_ONE) {
		settle_free_one(store, products, NULL, saveing, consume);
	}else if(type == DISCOUNT) {
		settle_discount(store, products, NULL, saveing, consume);
	}else{
		assert(!"no this promotion\n");
	}				

}

/*只享受买二送一优惠商品结算
*@parm: store商品库存
*@parm: free_one_set 购买的买二送一商品
*@parm: double_set 购买的多重优惠的商品
*@parm: saveing 节省金额总计
**/
void settle_free_one(struct set_t *store, struct set_t *free_one_set,\
				struct set_t *double_set, float *saveing,\
							float *consume)
{
	struct set_t *free_ones;
	int i;

	free_ones = NULL;
	if(double_set) { //去除享受多重优惠的商品
		free_ones = set_minus(free_one_set, double_set);
	}
	
	if(free_ones) { //去除享受多重优惠的商品
		set_map(free_ones, settle_free_one_print, (void *)store, consume, saveing);
		set_free(&free_ones);
	}else if(free_one_set){
		set_map(free_one_set, settle_free_one_print, (void *)store, consume, saveing);	
	}

}

/*买二送一活动计算，并打印小票信息
*@parm: member 商品条形码信息
*@parm: c 库存商品集合
*@parm: c1 总计金额
*@parm: c2 节省金额
*@parm: c3 赠送商品列表*/
void settle_free_one_print(const void *member, void *c, float *c1, float *c2)
{
	struct product_detail *product;
	struct sales_products *sales;
	unsigned int count;  //购买的商品数量
	float money; //小记金额
	int free_count; //赠送个数
	float price; //单价
	float save_money; //节省金额
	
	count = money = save_money = free_count = 0;
	sales = (struct sales_products *)member;
	product = (struct product_detail *)set_member((struct set_t *)c, sales);
	
	assert(product && sales);
	count = sales->sum;
	price = product->unit_price;

	while(count > 2 ) {
		count -= 3; 
		free_count++;
	}
	
	count = sales->sum;
	money = (count - free_count) * price;
	save_money = free_count * price;
	put_free_one_list(product->name, free_count);

	printf("名称:%s,",product->name);
	printf("数量:%u,",count);
	printf("单价:%.1f(元),",price);
	printf("小记:%.1f(元)\n", money);

	(*c1) += money; //更新消费总计
	(*c2) += save_money; //更新节省总结

}

static free_one_list put_free_one_list(char *name, unsigned int sum)
{
	static struct free_one_list *plist = NULL;
	struct free_one_list *p;

	if (!name && !sum) {
		return plist;
	}

	PNEW(p, 1, struct free_one_list*);
	p->name = name;
	p->count = sum;

	//链表头插法
	p->next = plist;
	plist = p;

	return plist;
}

/*只享受折扣的商品结算
*@parm: store商品库存
*@parm: discount_set 购买的折扣商品
*@parm: double_set 购买的多重优惠的商品
*@parm: saveing 节省金额总计
**/
void settle_discount(struct set_t *store, struct set_t *discount_set,\
			struct set_t *double_set, float *saveing, float *consume)
{

	struct set_t *discounts;
	int i;

	discounts = NULL;
	if(double_set) { //去除享受多重优惠的商品
		discounts = set_minus(discount_set, double_set);
	}
	
	if(discounts) { //去除享受多重优惠的商品
		set_map(discounts, settle_discount_print, (void *)store, consume, saveing);
		set_free(&discounts);
	}else if(discount_set){
		set_map(discount_set, settle_discount_print, (void *)store, consume, saveing);	
	}
}

/*折扣计算，并打印小票信息
*@parm: member 商品条形码信息
*@parm: c 库存商品集合
*@parm: c1 总计金额
*@parm: c2 节省金额*/
void settle_discount_print(const void *member, void *c, float *c1, float *c2)
{
	struct product_detail *product;
	struct sales_products *sales;
	unsigned int count;  //购买的商品数量
	float money; //小记金额
	float rate; //折扣率
	float price; //单价
	float save_money; //节省金额

	count = money = save_money = 0;
	sales = (struct sales_products *)member;
	product = (struct product_detail *)set_member((struct set_t *)c, sales);

	count = sales->sum;
	price = product->unit_price;
	money = (count * price) * DISCOUNT_RATE;
	save_money = (count * price) - money;
	assert(product && sales);
	printf("名称:%s,",product->name);
	printf("数量:%u,",count);
	printf("单价:%.1f(元),",price);
	printf("小记:%.1f(元)", money);
	printf("节省%.1f(元)\n",save_money);

	(*c1) += money; //更新消费总计
	(*c2) += save_money; //更新节省总结

}

/*将购物车中的商品加入到集合中*/
void pack_cart(int sum, char *bar_code, void *shoping_set)
{
	struct sales_products *one_product; //购物车商品
    	
	PNEW(one_product, 1, struct sales_products *);
	strcpy(one_product->bar_code, bar_code);
	one_product->sum = sum;
	set_put((struct set_t *)shoping_set, one_product);
}

static const char *skip(const char *str)
{
	while (str && *str && (unsigned char)*str <= 32)
		str++; 	
	return str;
}

/*根据传入的JSON数据,解析出商品条形码,数量,然后填充到集合中
*@parm: value JSON数据
*@parm:c1 集合结构
*@parm: apply 业务函数
**/
void prase_JSON_array(const char *value, void *c1,\
							void apply(int, char*, void *)) 
{
	int sum;      //商品数量
	char *item; //商品条形码

	sum = 0;
	value = skip(value); //跳过开头无用的字符	
	if (*value!='[') {
		return;
	}	/* not an array! */
	value++;
	value = skip(value); 
	if (*value==']')
		return;	/* empty array*/
	
	/*解析JSON, 将数据填充到数据结构中*/
	do {
		value = parse_value(value, &sum, &item);
		apply(sum, item, c1);          //将商品信息,填充到集合中
	} while(*(value = skip(value)) == ',' && value++);
	
	value = skip(value);
	assert(*value == ']'); //解析完毕,确保JSON格式正确
}

/*解析 'ITEMxxxxx-n', 'ITEMxxxxx'字符串
* 'ITEMxxxxx-n' 解析成 'ITEMxxxxx' 和 n 
* 'ITEMxxxxx'   解析成 'ITEMxxxxx' 和 0*/
static const char *parse_value(char const *item, int *n, char **value) 
{
	char c = '-';
	const char *pitem;
	const char  *p;
	char *val;  //存放'ITEMxxxxxx'字符串
	int number; //取得'ITEMxxxxx-n'中的'n'
	
	number = 0;
	pitem = item;
	pitem = skip(pitem);
	assert(*pitem == '\'');
	pitem++;  //跳过 单引号字符
	p = pitem; 
	while(isalnum(*pitem))
		pitem++;   //检测 '-'和'''字符终止循环

	if(*pitem == '\'') {  //‘ITEMxxxxx’格式
		PNEW(val, (pitem-p) + 1, char*);
		strncpy(val, p, pitem-p); //copy 条形码
		*n = 0;
		*value = val;
	}else if(*pitem == '-') {  // ‘ITEMxxxxx-n’格式
		
		PNEW(val, (pitem-p) + 2, char*);
		strncpy(val, p, pitem-p);
		
		pitem++; //跳过'-'字符
		while(*pitem >= '0' && *pitem <= '9') 
			number = (number * 10) + (*pitem++ - '0');
		*n = number;
		*value = val;
	}else {
		assert(!"JSON format error");
	}
	
	pitem++; //跳过最后一个单引号
	return pitem;
}

