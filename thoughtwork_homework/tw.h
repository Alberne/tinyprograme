#ifndef TW_H_
#define TW_H_

typedef struct product_detail{
	char bar_code[15];     // 商品条形码，也是商品ID 
	char name[15];     
	float unit_price;     //单价
	char  unit[8]; 
	char category[15];     //商品类别 
}*product_detail;

enum promotion_category {  //活动种类定义
	NONE = 0,  //不参见任何活动
	DISCOUNT,  //95折扣
	FREE_ONE     //买二送一
};

typedef struct sales_promotion{
	enum promotion_category priority;  //如果优惠活动冲突时，优先使用哪个优惠活动 
	struct promotion_one_info{
		enum promotion_category type; //促销活动，默认是0x00，不参加任何活动 
		char statement[50];   //促销活动名 
		struct set_t *sales_set; //促销活动的商品 
		struct promotion_one_info *next_promotion; //下一个优惠活动 
	}*promotion_all_info;
}*sales_promotion;

typedef struct sales_products{
	char bar_code[15];  //商品条形码 
	unsigned int sum;   //商品数量 
}*sales_products;

//买二送一活动,赠送的商品
typedef struct free_one_list{
	char *name;
	unsigned int count;
	struct free_one_list *next;
}*free_one_list;

#define array_len(ar) (sizeof(ar) / sizeof(ar[0]))  //ar必须是数组 
#define bar_code_len 15   //商品条形码的长度
#define DISCOUNT_RATE  0.95  //折扣率

/*内存分配器管理
* p：必须是指针，分配完后指向内存地址
* c：内存块的个数
* type:p的指针类型
**/
#define PNEW(p,c,type) do{\
		p = (type)calloc(c, sizeof(*(p)));\
		assert(p);\
}while(0) /*no trailing ;*/

/*---------------打印小票模块  ----------------------*/

void prase_JSON_array(const char *value, void *c1,\
							void apply(int, char*, void *)); //解析json格式中的数组，

static const char *parse_value(char const *item, int *n, char **value);//解析商品条形码
static const char *skip(const char *str); //utility to jump whitespace and cr/1f
void pack_cart(int sum, char *bar_code, void *shoping_set);  //将购物车中的商品加到集合中
static free_one_list put_free_one_list(char *name, unsigned int sum);

/*购物车商品结算业务模块
*@parm: store 库存商品集合
*@parm: promotions  优惠活动商品集合
*@parm: shopping 购物车商品集合
*/
void pay(struct set_t *store, struct sales_promotion *promotions, struct set_t *shopping);

/*只享受折扣的商品结算
*@parm: store商品库存
*@parm: discount_set 购买的折扣商品
*@parm: double_set 购买的多重优惠的商品
*@parm: saveing 节省金额总计
**/
void settle_discount(struct set_t *store, struct set_t *discount_set,\
                        struct set_t *double_set, float *saveing, float *consume);


/*折扣计算，并打印小票信息
*@parm: member 商品条形码信息
*@parm: c 库存商品集合
*@parm: c1 总计金额
*@parm: c2 节省金额*/
void settle_discount_print(const void *member, void *c, float *c1, float *c2);


/*只享受买二送一优惠商品结算
*@parm: store商品库存
*@parm: free_one_set 购买的买二送一商品
*@parm: double_set 购买的多重优惠的商品
*@parm: saveing 节省金额总计
**/
void settle_free_one(struct set_t *store, struct set_t *free_one_set,\
							struct set_t *double_set, float *saveing,\
								float *consume);

/*买二送一活动计算，并打印小票信息
*@parm: member 商品条形码信息
*@parm: c 库存商品集合
*@parm: c1 总计金额
*@parm: c2 节省金额*/
void settle_free_one_print(const void *member, void *c, float *c1, float *c2);

/*多重优惠计算
*@parm: store商品库存
*@parm: priority  哪种优惠优先
*@parm: products 购买的多重优惠商品
*@parm: saveing 节省金额总计
*@parm: consume 消费金额总计
**/
void settle_double_promotion(struct set_t *store, enum promotion_category type,
							struct set_t *products, float *saveing,\
								float *consume);

/*没有优惠的商品结算
*@parm: store 库存商品
*@parm: products 购物车中不享受优惠的商品
*@parm: saveing 节省总计
*@parm: consume 消费总计*/
void settle_product(struct set_t *store, struct set_t *products,\
					 float *saveing, float *consume);

/* 不享受优惠活动的商品计算，并打印小票
*@parm: member 商品条形码信息
*@parm: c 库存商品集合
*@parm: c1 总计金额
*@parm: c2 节省金额
**/
void settle_product_print(const void *member, void *c, float *c1,\
						  float *c2);

/*赠送商品列表打印*/
void free_one_print(free_one_list list);
/*小票末尾信息打印*/
void print_tail(float saveing, float consume);

/*---------------打印小票模块 END---------------------*/

#endif


