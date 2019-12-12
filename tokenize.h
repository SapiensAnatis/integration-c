#ifndef RPN_H_INCLUDED
#define RPN_H_INCLUDED // Include guards: block the same header from being included twice in a file

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h> // type references
#include "token.h"
#include "stack.h"

// Shunting yard/RPN
int exp_to_tokens(char*, struct Token*);
void print_tokenized(struct Token *, int);
void print_token(struct Token*);
void compile_regex(char*, pcre2_code**);

#endif