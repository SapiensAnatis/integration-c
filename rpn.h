#ifndef RPN_H_INCLUDED
#define RPN_H_INCLUDED // Include guards: block the same header from being included twice in a file

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h> // type references
// Shunting yard/RPN

enum Token_Type;
enum Operator_Type;
enum Function_Type;
enum Associativity;
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

#endif