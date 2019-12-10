#ifndef RPN_H_INCLUDED
#define RPN_H_INCLUDED

void FormatExpression(char *expression);
char* InfixToRPN(char *expression);
double EvaluateRPN(char *rpn_expression, double x);

#endif