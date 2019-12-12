#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shunting.h"
#include "token.h"
#include "stack.h"

// Attributions: Adapted from pseudocode at 
// https://en.wikipedia.org/wiki/Shunting-yard_algorithm#The_algorithm_in_detail

// Function: shunting_yard(input_tokenized, token_count, output_tokenized)
// Description: Performs Dijkestra's shunting-yard algorithm on a tokenized infix expression to
//              generate a tokenized RPN expression
// Parameters: input_tokenized, a ptr to the start of an array where tokens are stored backwards
//             token_count, an integer equal to the number of tokens in the array
//             output_tokenized, a ptr to the start of an array where the output should be stored
// Outputs: An integer corresponding to the number of tokens in the final expression (which is not
//          equal to token_count, because parentheses are removed). This value is -1 if mismatched
//          parentheses were detected. It will also return -2 if the program attempted to access
//          the top of the operator stack when it was empty, i.e. if the expression contains an
//          operator first.

int shunting_yard(struct Token *input_tokenized, 
                            int token_count, 
                            struct Token *output_tokenized) {
    // Initialize operator stack
    printf("Allocating operator stack...");
    struct Stack *op_stack = init_stack(token_count);
    // Initialize output stack
    printf("Allocating output stack.....");
    struct Stack *ret_stack = init_stack(token_count);
    // counter for output stack length - hard to know once written to memory
    int output_token_count;
    // Top of operator stack - so we don't have to access it every single time
    struct Token *op_stack_top = get_stack_start(op_stack);

    // remember the tokenized input is backwards, so compensate for that
    for (int i = token_count-1; i >=0; i--) { 
        // Loop through all the tokens to be read in
        struct Token *token = (input_tokenized + i);

        // What type of token is it?
        if (token->type == Number) {
            push_stack(ret_stack, *token);
        } else if (token->type == Function) {
            push_stack(op_stack, *token);
        } else if (token->type == Operator) {
            // Check we aren't looking at an empty stack (this means an operator was put first,
            // for instance *4*4 isn't a valid expression)
            printf("[Shunting] Operator found. Operator stack top: %p\n", op_stack_top);
            if (is_stack_empty(op_stack) || op_stack_top == NULL) { push_stack(op_stack, *token); continue; }
            // I'm really sorry about this boolean check
            while (
                (
                    (
                        op_stack_top->type == Function
                    ) ||
                    (
                        op_stack_top->type == Operator && 
                        op_stack_top->precedence > token->precedence
                    ) ||
                    (
                        op_stack_top->type == Operator && 
                        op_stack_top->precedence == token->precedence &&
                        op_stack_top->associativity == Assoc_Left
                    ) 
                ) && op_stack_top->type != Bracket_Left
            ) { 
                push_stack(ret_stack, pop_stack(op_stack)); 
                op_stack_top = get_stack_top(op_stack); // otherwise an infinite loop results
            }

            push_stack(op_stack, *token);
        } else if (token->type == Bracket_Left) {
            push_stack(op_stack, *token);
        } else if (token->type == Bracket_Right) {
            while (op_stack_top->type != Bracket_Left) {
                push_stack(ret_stack, pop_stack(op_stack));
                op_stack_top = get_stack_top(op_stack);
            }
            // Once that loop is done, the operator stack will either be empty or have a left 
            // parentheses on top. If it's empty, that means there are mismatched parentheses.
            if (is_stack_empty(op_stack)) {
                return -1; // error return code (rc)
            } else if (op_stack_top->type == Bracket_Left) {
                pop_stack(op_stack); // Discard top
                op_stack_top = get_stack_top(op_stack);
            }
        }

        // Update top stack pointer
        printf("[Shunting] (i = %d) Finished evaluating token of type %d\n", i, token->type);
        printf("[Shunting] Getting new stack top: Stack start (%p), Stack top (%p), ",
                op_stack->start, op_stack_top);
        op_stack_top = get_stack_top(op_stack);
        printf("New stack top (%p)\n", op_stack_top);
    }

    // Pop remainder of operator stack onto output queue
    printf("Popping contents of operator stack into output stack...\n");
    while (!is_stack_empty(op_stack)) {
        if (
            op_stack_top->type == Bracket_Left || 
            op_stack_top->type == Bracket_Right
        ) { // There were mismatched parentheses
            return -1;
        } else {
            push_stack(ret_stack, pop_stack(op_stack));
        }
    }

    // Write output stack to memory location given
    int j = 0;
    while (!is_stack_empty(ret_stack)) {
        struct Token to_write = pop_stack(ret_stack);
        memcpy(output_tokenized + j, &to_write, sizeof(to_write));
        j++;
    }

    return j; // rc for success: number of tokens in the final rpn expression
}