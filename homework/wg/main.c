#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "table.h"
#include "main.h"
#include <string.h>
 

#define electron "电子"
#define commdity "日用品"
#define food "食物"
#define loquor "酒类"

/*内存分配器管理
* p：必须是指针，分配完后指向内存地址
* c：内存块的个数
* type:p的指针类型
**/
#define PNEW(p,c,type) do{\
		p = (type)calloc(c, sizeof(*(p)));\
		assert(p);\
}while(0) /*no trailing ;*/

table_t get_store() 
{
	table_t store; //商店库存商品
	int row, column;	
	static goodStyle type[] = {ELECTRON, COMMODITY, FOOD, LIQUOR};
	static char* goods[4][5] = {
		{"ipad", "iphone", "显示器", "笔记本电脑", "键盘"}, //ELECTRON
		{"面包", "饼干", "蛋糕", "鱼", "蔬菜"},          //FOOD
		{"餐巾纸", "收纳箱", "咖啡杯", "雨伞"},          //COMMODITY
		{"啤酒", "白酒", "伏特加"}                      //LIQUOR
	};
	
	store = table_new(512, NULL, NULL);
	for(row = 0; row < 4; row++) {
		for(column = 0; column < 5 ; column++) {
			if(goods[row][column])
				table_put(store, goods[row][column], &type[row]);	
		}
	}
	return store;
} 
//返回购物车中结算的商品 
struct cart_info *get_shopping(char *filename)
{
	struct cart_info *products; 
	FILE *fp;
	int len;
	static char temp[200]; 
	
	PNEW(products, 1, struct cart_info *);
	products->promts = NULL;
	products->items = NULL;
	products->one_coupon = NULL;
	 
	len = sizeof(temp);
	memset(temp, 0, len);
	fp = fopen(filename, "r");
	if(fp){
		//skip(fp); //跳过文件开头空白字符 
		do{
			memset(temp, 0, len);
			fgets(temp, len, fp); //读取文件一行 
			anlaysis_text(temp, products);	
		}while(!(feof(fp) || ferror(fp)));
		
	}
	return products;
}


void anlaysis_text(char *str, struct cart_info *cart) {
	static int length = 1;
	static int flag = 1;
	struct promotion *prots; //折扣信息 
	struct shopping *shopps;  //商品信息
	struct coupon *coup;    //优惠券
	
	if((strlen(str) == 1) && (length != 1))
		flag++; //上行文本非空,且这行为空,下一类信息出现 
	
	length = strlen(str);
	if(length == 1 || length == 0)
	 	return; //空行不需要解析 
	 	
	switch(flag) {
		case 1:    //折扣信息 
			printf("%s\n",str); 
			prots = convert_promotion(str);
			prots->next = cart->promts;
			cart->promts = prots;
			break;
		case 2:    //购物商品 
			printf("%s",str); 
			shopps = convert_shopping(str);
			shopps->next = cart->items;
			cart->items = shopps;
			break;
		case 3:   //结算日期
			printf("%s\n",str);
			cart->settle_date = convert_time(&str);
			break;
		case 4:  //优惠券信息 
			printf("%s\n",str); 
			if(!cart->one_coupon) {
				cart->one_coupon = convert_coupon(str);
			}else {
				printf("coupon always exist, you should use one coupon\n");
			}
			break;
		defalut:
			assert(!"input format error\n");
			break;	
	}
	
}

//将字符串转化成优惠券信息结构体
struct coupon *convert_coupon(char *str)
{
	struct coupon *coup;
	PNEW(coup, 1, coupon);
	coup->date = convert_time(&str);
	sscanf(str, "%f %f", &coup->threshold, &coup->privilege);
	return coup;

}

//将字符串转化成商品信息结构体
struct shopping *convert_shopping(char *str)
{	
	struct shopping *shop;
	char temp[15];
	int i;
	memset(temp, 0, sizeof(temp));
	PNEW(shop, 1, shopping);
	while(*str && (unsigned int)(*str) <= 32)
		str++; //跳过空格
	for(i = 0; isdigit(*str); str++)
		temp[i++] = *str;
	shop->sum = atoi(temp);
	while(*str && (unsigned int)(*str) <= 32)
		str++; //跳过空格
	assert(*str++ == '*');
	while(*str && (unsigned int)(*str) <= 32)
		str++; //跳过空格

	memset(temp, 0, sizeof(temp));
	for(i = 0; *str != ':'; str++)
		temp[i++] = *str;
	strcpy(shop->name, temp);
	while(*str && (unsigned int)(*str) <= 32)
		str++; //跳过空格
	assert(*str++ == ':');
	while(*str && (unsigned int)(*str) <= 32)
		str++; //跳过空格
	memset(temp, 0, sizeof(temp));
	
	for(i = 0; isdigit(*str); str++)
		temp[i++] = *str;
	
	shop->amount = atof(temp);
	return shop;

}



/*将字符串信息转化成折扣信息的结构体*/ 
struct promotion *convert_promotion(char *str)
{

	char temp[15];
	int i;
	struct promotion *promot;
	PNEW(promot, 1, promotion);
	promot->date = convert_time(&str);
	memset(temp, 0, sizeof(temp));
	while(*str && (unsigned int)(*str) <= 32)
		str++; //跳过空格
	assert(*str++ == '|');
	for(i = 0; isdigit(*str) || *str == '.'; str++)
		temp[i++] = *str;
	assert(*str++ == '|');
	promot->rate = atof(temp);
	promot->type = get_goodstyle(str);
	return promot;	
	
}

goodStyle get_goodstyle(char *str)
{
	char temp[10];
	char *p;
	
	memset(temp, 0, sizeof(temp));

	while(*str && (unsigned int)(*str) <= 32)
		str++; //跳过空格
	
	p = str;
	while(*str != '\n')
		str++;
	*str = '\0';
	strcpy(temp, p);

	if(strcmp(temp,electron) == 0) {
		return ELECTRON;
	}else if(strcmp(temp,commdity) == 0) {
		return COMMODITY;
	}else if(strcmp(temp,food) == 0) {
		return FOOD;
	}else if(strcmp(temp, loquor) == 0) {
		return LIQUOR;
	}else {
		assert(!"goodStyle error");	
	}
		

}

/*'2011.11.11 转化成time_t结构体'*/
time_t convert_time(char **p)
{
	time_t t;
	struct tm *date;
	int i;
	char temp[6];
	char *str;
	str = *p;
	
	i = 0;
	memset(temp, 0, sizeof(temp));
	time(&t);
	date = localtime(&t);
	memset(date, 0, sizeof(*date));
	//跳过字符串空格
	while(*str && (unsigned int)(*str) <= 32)
		str++;
			
	for(i = 0; isdigit(*str); i++)
		temp[i] = *str++; //获得年份
	date->tm_year = atoi(temp) - 1900;	 
	memset(temp, 0, sizeof(temp));
	assert(*str++ == '.');
	
	for(i = 0; isdigit(*str); i++)
		temp[i] = *str++; //获得月份 
	date->tm_mon = atoi(temp) - 1;	
	memset(temp, 0, sizeof(temp)); 	
	assert(*str++ == '.');
	
	for(i = 0; isdigit(*str); i++)
		temp[i] = *str++; //获得号
	date->tm_mday = atoi(temp);	
	t = mktime(date);

	assert(t != -1);
	*p = str;

	return t;	 
} 
 
//跳过文件开头的空字符,以及换行符 
static void skip(FILE *fp)
{
	char c;
	c = getc(fp);
	while(c && (unsigned char)c <= 32)
		c = getc(fp);
}


void pay(struct cart_info *cart, table_t goods)
{
	goodStyle *category;
	float consume;
	float money;
	float rate;
	struct shopping *product;
	struct coupon *coup;
	double diff;

	money = consume = rate = 0;
	product = cart->items;
	coup = cart->one_coupon;

	for( ;product ; product = product->next) {  //循环商品列表,计算折扣
		category = table_get(goods, product->name);	
		rate = get_discount(cart->promts, cart->settle_date, *category);
		money = rate * product->amount;
		consume += money;

	}

	//清算优惠券
	if(coup) {
		diff = difftime(cart->settle_date, coup->date);
		if(consume > (coup->threshold) && diff < 0)
			consume -= coup->privilege;	
	}
	//打印结算金额 
	printf("\n\n%.2f\n==========================\n", consume);
	

}

float get_discount(struct promotion *promos, time_t settle_date, goodStyle type)
{
	float discount;
	promotion p;
	double diff;
	discount = 1; //没有优惠
	
	for(p = promos; p; p = p->next) {
		diff = difftime(settle_date, promos->date);
		if(p->type == type && !diff)
			return p->rate;
	
	}
	return discount;

}

int main(int argc, char *argv[]) {
	table_t goods;
	struct cart_info *shopping;
	if(argc != 2) {
		printf("usage: <this programe name>  <input file name>\n");
		return -1;	
	}
	goods = get_store();
	shopping = get_shopping(argv[1]);
	pay(shopping, goods);
	table_free(goods);
	return 0;
}

void pay_for(char *filename)
{
	table_t goods;
	struct cart_info *shopping;
	if(!filename) {
		printf("please input file name\n");
		return;	
	}
	goods = get_store();
	shopping = get_shopping(filename);
	pay(shopping, goods);
	return ;

}


