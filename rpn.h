#ifndef RPN_H_INCLUDED
#define RPN_H_INCLUDED // Include guards: block the same header from being included twice in a file

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h> // type references
// Shunting yard/RPN

enum Token_Type;
enum Operator_Type;
enum Function_Type;
enum Associativity;
struct Token;

int exp_to_tokens(char*, struct Token*);
void print_tokenized(struct Token*, int);
void print_token(struct Token*);
void compile_regex(char*, pcre2_code**);
struct Stack* infix_to_RPN(char*);
double evaluate_RPN(char*, double);

// Stacks

struct Stack;
struct Stack *init_stack(int);
void push_stack(struct Stack*, struct Token);
struct Token pop_stack(struct Stack*);
struct Token *get_stack_top(struct Stack*);
int is_stack_empty(struct Stack*);
int get_stack_size(struct Stack*);
void delete_stack(struct Stack*);

#endif