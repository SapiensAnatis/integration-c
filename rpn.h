#ifndef RPN_H_INCLUDED
#define RPN_H_INCLUDED // Include guards: block the same header from being included twice in a file

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h> // type references
// Shunting yard/RPN

enum Token_Type {
    Operator, 
    Number,
    Variable,
    Bracket_Left, // having a separate enum for bracket and (left, right) seems a bit silly to me
    Bracket_Right,
    Function
};

enum Operator_Type {
    Op_Add,
    Op_Subtract,
    Op_Multiply,
    Op_Divide,
    Op_Power
};

enum Function_Type {
    Func_Sin,
    Func_Cos,
    Func_Tan,
    Func_Ln,
    Func_Exp,
    Func_Log
};


enum Associativity {
    Assoc_Left,
    Assoc_Right
};

struct Token {
    enum Token_Type type;
    double value; 
    enum Operator_Type operator_type;
    int precedence;
    enum Associativity associativity;
    enum Function_Type function_type;
};

int exp_to_tokens(char*, struct Token*);
void print_tokenized(struct Token*, int);
void print_token(struct Token*);
void compile_regex(char*, pcre2_code**);
struct Stack* infix_to_RPN(char*);
double evaluate_RPN(char*, double);

// Stacks

#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED // Include guards: block the same header from being included twice in a file

struct Stack;
struct Stack *init_stack(int);
void push_stack(struct Stack*, struct Token);
struct Token pop_stack(struct Stack*);
struct Token *get_stack_top(struct Stack*);
int is_stack_empty(struct Stack*);
int get_stack_size(struct Stack*);
void delete_stack(struct Stack*);

#endif

#endif