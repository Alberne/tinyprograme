#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "set.h"


#define array_len(ar) (sizeof(ar) / sizeof(ar[0]))  //ar���������� 

/*�ڴ����������
* p��������ָ�룬�������ָ���ڴ��ַ
* c���ڴ��ĸ���
* type:p��ָ������
**/
#define PNEW(p,c,type) do{\
		p = (type)calloc(c, sizeof(*(p)));\
		assert(p);\
}while(0) /*no trailing ;*/

typedef struct product_detail{
	char bar_code[15];     // ��Ʒ�����룬Ҳ����ƷID 
	char name[15];     
	float unit_price;     //����
	char  unit[8]; 
	char category[15];     //��Ʒ��� 
}*product_detail;

typedef struct sales_promotion{
	char priority;  //����Żݻ��ͻʱ������ʹ���ĸ��Żݻ 
	struct promotion_one_info{
		char promotion_type; //�������Ĭ����0x00�����μ��κλ 
		char statement[50];   //������� 
		struct set_t *sales_set; //���������Ʒ 
		struct promotion_one_info *next_promotion; //��һ���Żݻ 
	}*promotion_all_info;
}*sales_promotion;

typedef struct sales_products{
	char bar_code[15];  //��Ʒ������ 
	unsigned int sum;   //��Ʒ���� 
}*sales_products;


//ͨ���Ƚ���Ʒ�������룬�ж��Ƿ�Ϊͬһ��Ʒ 
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

static struct set_t *get_products(void);
void setting_promotion();//�����Żݻ  

int main(int argc, char *argv[]) {
	
	//������ϵͳ������Ʒ������� 
	struct set_t *products; //�����Ʒ���� 
	products = get_products();
	 
	//�����Żݻ��Ϣ
	setting_promotion();
    
     
	 
	




	return 0;
}

//�����Żݻ 
void setting_promotion() 
{
    static struct sales_promotion *promotions;
    struct promotion_one_info *p; //��ʱ����  
    struct set_t *pset; //��ʱ����
     
    PNEW(promotions, 1, struct sales_promotion *);
    puts("--------ͨ������ϵͳ�����Żݻ--------\n");
    /*���赱ǰ���������ֻ*
    *  1. �����һ�
    *  2. 9.5���Ż� */
    
     promotions->priority = 1;  //��ĳ����Ʒ���������Ż�ʱ������ѡ��һ
     
     /*�������û1��Ϣ*/
     PNEW(promotions->promotion_all_info, 1,\
                        struct promotion_one_info *);
     p = promotions->promotion_all_info;
     pset = p->sales_set; 
     p->promotion_type = 1;
     strcpy(p->statement, "�����һ�");
     pset = set_new();
 
     /*������û2��Ϣ*/ 
     
}

/*������ϵͳȡ����Ʒ��Ϣ*/ 
static struct set_t *get_products(void)
{
    static struct set_t *product_set;
    int i;
	static struct product_detail products[] = {
		{"ITEM00001", "����", 10, "��", "ˮ��"},
		{"ITEM00002", "ƻ��", 100, "��", "ˮ��"},
		{"ITEM00003", "�㽶", 3.5, "��", "ˮ��"},
		{"ITEM00004", "����", 10.5, "��", "ˮ��"},
		{"ITEM00005", "����", 99, "��", "�ɹ�"},
		{"ITEM00006", "����", 19.5, "��", "�ɹ�"},
		{"ITEM00007", "���Ĺ�", 10.5, "��", "�ɹ�"},
		{"ITEM00008", "����", 10.5, "��", "�ɹ�"}
	};
	
	/*��ʼ����Ʒ��Ϣ����*/
	product_set = set_new(1024, compare_product, hashcode);
    for(i = 0; i < array_len(products); i++) {
        set_put(product_set, &products[i]);
    }
    puts("��Ʒ��Ϣ�������\n");
    return product_set;
}

