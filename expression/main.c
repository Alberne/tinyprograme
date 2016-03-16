#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "list.h"

#define DEBUG 0
#define OPERAND_LEN  15  //操作数字节数 
#define OPERATOR_LEN 2   //操作符自己数 

#define PNEW(p, n) do{\
	p = calloc(n, sizeof(*(p)));\
	assert(p);\
	}while(0) /*no trail ;*/

typedef enum bool{
	FALSE = 0,
	TRUE = 1
}bool;
//确定操作符的优先级
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

/*将中缀表达式转换成后缀表达式 
*@parm: sexpr 中缀表达式
*return: 存储后缀表达式的队列  
**/
List convert_postfix_expression(char *expr)
{
	List queue_exp; //后缀表达式队列,也作为操作数队列 
	List stack_opr;       //运算符栈 
	char temp[25];
	int i;
	char *pstr, *p;
	
	queue_exp = stack_opr = NULL;
	while(*expr != '\0') { //扫描整个中缀表达式 
		//isoperand = FALSE;
		i = 0; 
		
		if(isdigit(*expr) || *expr == '.') { //是操作数(实数) 
			memset(temp, 0, sizeof(temp));
			while(isdigit(*expr) || *expr == '.') {  //考虑操作数是多位的情况 
				temp[i++] = *expr++; 
			}/*while*/
			PNEW(pstr, i);
			strcpy(pstr, temp);
			queue_exp = list_push(queue_exp, pstr); //操作数入队列 
		
		}else { //是运算符 
			PNEW(pstr, OPERATOR_LEN);
			
			switch(*expr) {
				case '(':   //左括号直接压入操作符栈 
					*pstr = '(';
					stack_opr = list_push(stack_opr, pstr);
					break;
					
				case '+':
				case '-':
				case '*':
				case '/': 
					if(list_length(stack_opr)) {//操作符优先级比较 
						//当前运算符,小于栈顶运算符 
						while(priority(*expr) <= priority(*((char *)stack_opr->ptr))) {
							stack_opr = list_pop(stack_opr, &p); //取出优先级高的栈顶运算符
							queue_exp = list_push(queue_exp, p); //入队 
							
							if(list_length(stack_opr) == 0) 
								break; //如果栈顶为空,退出while 
						}/*while*/
					}/*if*/
					
					//当前运算符进栈 
					*pstr = *expr;
					stack_opr = list_push(stack_opr, pstr);
					break;
					
				case ')':
					/*取出运算符栈中左小括号以
					上得全部运算符压入操作符队列中 */
					while(*((char *)stack_opr->ptr) != '(') {
						stack_opr = list_pop(stack_opr, &p);  //出栈 
						queue_exp = list_push(queue_exp, p); //入队 
						
					}/*while*/
					
					stack_opr = list_pop(stack_opr, &p); //丢掉'(' 
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
	
	//将操作符全部压入操作数队列中
	while(list_length(stack_opr)) {
		stack_opr = list_pop(stack_opr, &p);  //出栈 
		queue_exp = list_push(queue_exp, p); //入队
	} 
	#if DEBUG
	print_list(queue_exp, "queue");
	print_list(stack_opr, "stack");
	#endif
		
	return queue_exp;
}




/*计算后缀表达式的值*/

float caculate(List queue_postfix)
{
	List stack_sum;  //栈，存储操作数 
	float f1, f2; //二元运算符两边的操作数 
	char *p, *psum;
	
	psum = p = NULL;
	stack_sum = NULL;
	f1 = f2 = 0;
	
	#if DEBUG
	print_list(queue_postfix, "postfix_queue");
	#endif
	
	//逆置队列后可以使用pop()方法取队尾元素 
	queue_postfix = list_reverse(queue_postfix); 
	
	#if DEBUG
	print_list(queue_postfix, "postfix_queue");
	#endif
	
	//循环遍历队列元素 
	while(list_length(queue_postfix)) {
		queue_postfix = list_pop(queue_postfix, &p); //取出队尾元素
		
		if((isdigit(*p)) || (*p == '.')) { //操作数 
			stack_sum = list_push(stack_sum, p);  //压栈 
		}else { //运算符 
			stack_sum = list_pop(stack_sum, &psum);
			f2 = atof(psum);  //参与运算的第二个操作数 
			free(psum);
			psum = NULL;
			stack_sum = list_pop(stack_sum, &psum);
			f1 = atof(psum);  //参与运算的第一个操作数
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
						assert(!"div by zero error\n");  //除0错误 
					
					f1 /= f2; 
					break;
				
				default:
					assert(!"expression fromat erorr\n");
					break;						
			}/*switch*/ 
			 PNEW(psum, OPERAND_LEN);
			 sprintf(psum, "%f", f1);
			 stack_sum = list_push(stack_sum, psum); //将计算结果压栈 
		}/*else*/ 
					 
	}/*while*/
	stack_sum = list_pop(stack_sum, &psum); //将最后的计算结果出栈
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

