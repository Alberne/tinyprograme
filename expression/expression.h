#ifndef EXPRESSION_H
#define EXPRESSION_H
#include <stdarg.h>

/*用户自定义函数表达式计算
*@parm:  exp 表达式  eg: sum(1,2)/3 (1+2)*4/7-9
*@parm:  ... 可变参数列表 ，自定义函数的计算结果
**/
void calculate_exp(char *exp, ...);

#endif
