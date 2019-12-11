#ifndef RPN_H_INCLUDED
#define RPN_H_INCLUDED

// Shunting yard/RPN

enum TokenType;
struct Token;

Token* ExpressionToTokens(char*);
Token* InfixToRPN(char*);
double EvaluateRPN(char*, double);

int TokenIsOperator(char);
int TokenIsNumber(char);
int TokenIsFunction(char*);

// Stacks

struct Stack;
void PushStack(struct Stack*, char);
char PopStack(struct Stack*);
void DeleteStack(struct Stack*);

#endif