#ifndef SHUNTING_H_INCLUDED
#define SHUNTING_H_INCLUDED // Include guards

#include "stack.h"
#include "token.h"

int shunting_yard(struct Token *input_ptr, int token_count, struct Token *output_ptr);
void refresh_op_stack_top(struct Token *stack_top_ptr, struct Stack *stack);
double evaluate_rpn(struct Token *input_rpn, int num_tokens, double x);

#endif