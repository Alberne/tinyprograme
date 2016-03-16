#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "list.h"

#define DEBUG 0
#define OPERAND_LEN  15  //�������ֽ��� 
#define OPERATOR_LEN 2   //�������Լ��� 

#define PNEW(p, n) do{\
	p = calloc(n, sizeof(*(p)));\
	assert(p);\
	}while(0) /*no trail ;*/

typedef enum bool{
	FALSE = 0,
	TRUE = 1
}bool;
//ȷ�������������ȼ�
int priority(char opr)
{
	switch(opr)
	{
		case '(':
			return 0;
		case '-':
		case '+':
			return 1;
		case '*':
		case '/':
			return 2;
		default:
			assert(!"expression format error\n");
	}
} 

void print_list(List list, char *msg)
{
	char **ar;
	ar = list_to_array(list, NULL);
	printf("\n %s:", msg);
	for(; *ar; **ar++)
		printf("%s,", *ar); 
}

/*����׺���ʽת���ɺ�׺���ʽ 
*@parm: sexpr ��׺���ʽ
*return: �洢��׺���ʽ�Ķ���  
**/
List convert_postfix_expression(char *expr)
{
	List queue_exp; //��׺���ʽ����,Ҳ��Ϊ���������� 
	List stack_opr;       //�����ջ 
	char temp[25];
	int i;
	char *pstr, *p;
	
	queue_exp = stack_opr = NULL;
	while(*expr != '\0') { //ɨ��������׺���ʽ 
		//isoperand = FALSE;
		i = 0; 
		
		if(isdigit(*expr) || *expr == '.') { //�ǲ�����(ʵ��) 
			memset(temp, 0, sizeof(temp));
			while(isdigit(*expr) || *expr == '.') {  //���ǲ������Ƕ�λ����� 
				temp[i++] = *expr++; 
			}/*while*/
			PNEW(pstr, i);
			strcpy(pstr, temp);
			queue_exp = list_push(queue_exp, pstr); //����������� 
		
		}else { //������� 
			PNEW(pstr, OPERATOR_LEN);
			
			switch(*expr) {
				case '(':   //������ֱ��ѹ�������ջ 
					*pstr = '(';
					stack_opr = list_push(stack_opr, pstr);
					break;
					
				case '+':
				case '-':
				case '*':
				case '/': 
					if(list_length(stack_opr)) {//���������ȼ��Ƚ� 
						//��ǰ�����,С��ջ������� 
						while(priority(*expr) <= priority(*((char *)stack_opr->ptr))) {
							stack_opr = list_pop(stack_opr, &p); //ȡ�����ȼ��ߵ�ջ�������
							queue_exp = list_push(queue_exp, p); //��� 
							
							if(list_length(stack_opr) == 0) 
								break; //���ջ��Ϊ��,�˳�while 
						}/*while*/
					}/*if*/
					
					//��ǰ�������ջ 
					*pstr = *expr;
					stack_opr = list_push(stack_opr, pstr);
					break;
					
				case ')':
					/*ȡ�������ջ����С������
					�ϵ�ȫ�������ѹ������������� */
					while(*((char *)stack_opr->ptr) != '(') {
						stack_opr = list_pop(stack_opr, &p);  //��ջ 
						queue_exp = list_push(queue_exp, p); //��� 
						
					}/*while*/
					
					stack_opr = list_pop(stack_opr, &p); //����'(' 
					free(p);
					break;
					
				default :
					printf("----------%c\n",*expr);
					assert(!"expression format is error\n");
					break; 
				 
			}/*switch*/ 
			
			expr++;		
		}/*else*/
		#if DEBUG
		print_list(queue_exp, "queue");
		print_list(stack_opr, "stack");
		#endif
	}/*while*/
	
	//��������ȫ��ѹ�������������
	while(list_length(stack_opr)) {
		stack_opr = list_pop(stack_opr, &p);  //��ջ 
		queue_exp = list_push(queue_exp, p); //���
	} 
	#if DEBUG
	print_list(queue_exp, "queue");
	print_list(stack_opr, "stack");
	#endif
		
	return queue_exp;
}




/*�����׺���ʽ��ֵ*/

float caculate(List queue_postfix)
{
	List stack_sum;  //ջ���洢������ 
	float f1, f2; //��Ԫ��������ߵĲ����� 
	char *p, *psum;
	
	psum = p = NULL;
	stack_sum = NULL;
	f1 = f2 = 0;
	
	#if DEBUG
	print_list(queue_postfix, "postfix_queue");
	#endif
	
	//���ö��к����ʹ��pop()����ȡ��βԪ�� 
	queue_postfix = list_reverse(queue_postfix); 
	
	#if DEBUG
	print_list(queue_postfix, "postfix_queue");
	#endif
	
	//ѭ����������Ԫ�� 
	while(list_length(queue_postfix)) {
		queue_postfix = list_pop(queue_postfix, &p); //ȡ����βԪ��
		
		if((isdigit(*p)) || (*p == '.')) { //������ 
			stack_sum = list_push(stack_sum, p);  //ѹջ 
		}else { //����� 
			stack_sum = list_pop(stack_sum, &psum);
			f2 = atof(psum);  //��������ĵڶ��������� 
			free(psum);
			psum = NULL;
			stack_sum = list_pop(stack_sum, &psum);
			f1 = atof(psum);  //��������ĵ�һ��������
			free(psum);
			psum = NULL;
			
			switch(*p) {
				case '+':
					f1 += f2;
					break;
					
				case '-':
					f1 -= f2;
					break;
					  
				case '*':
					f1 *= f2;
					break;
					
				case '/':
					if(f2 == 0)
						assert(!"div by zero error\n");  //��0���� 
					
					f1 /= f2; 
					break;
				
				default:
					assert(!"expression fromat erorr\n");
					break;						
			}/*switch*/ 
			 PNEW(psum, OPERAND_LEN);
			 sprintf(psum, "%f", f1);
			 stack_sum = list_push(stack_sum, psum); //��������ѹջ 
		}/*else*/ 
					 
	}/*while*/
	stack_sum = list_pop(stack_sum, &psum); //�����ļ�������ջ
	return atof(psum);

} 

int main(int argc, char *argv[]) {
	List list;
	float ret;
	static char *ex = "(1+2.1)*3.5"; 
	list = convert_postfix_expression(ex);
	ret = caculate(list);
	printf("%s = %.2f\n",ex, ret);
	return 0;
}

