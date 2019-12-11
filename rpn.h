#ifndef RPN_H_INCLUDED
#define RPN_H_INCLUDED

// Shunting yard/RPN

enum TokenType;
struct Token;

Token* exp_to_tokens(char*);
void infix_to_RPN(char*);
double evaluate_RPN(char*, double);

// Stacks

struct Stack;
void push_stack(struct Stack*, char);
char pop_stack(struct Stack*);
int get_stack_top(struct Stack*);
int is_stack_empty(struct Stack*);
void delete_stack(struct Stack*);

#endif