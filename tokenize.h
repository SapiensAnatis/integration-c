#ifndef RPN_H_INCLUDED
#define RPN_H_INCLUDED // Include guards: block the same header from being included twice in a file

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h> // type references
#include "token.h"
#include "stack.h"

int exp_to_tokens(char *input_exp, struct Token *output_token_arr_ptr);
void compile_regex(char *regex_str, pcre2_code **output_re);

#endif