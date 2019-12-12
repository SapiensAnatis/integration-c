#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
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
    // printf("Allocating operator stack...");
    struct Stack *op_stack = init_stack(token_count);
    // Initialize output stack
    // printf("Allocating output stack.....");
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
        if (token->type == Number || token->type == Variable) {
            push_stack(ret_stack, *token);
        } else if (token->type == Function) {
            push_stack(op_stack, *token);
        } else if (token->type == Operator) {
            // Check we aren't looking at an empty stack (this means an operator was put first,
            // for instance *4*4 isn't a valid expression)
            // printf("[Shunting] Operator found. Operator stack top: %p\n", op_stack_top);
            if (is_stack_empty(op_stack) || op_stack_top == NULL) { 
                // printf("Operator stack was empty. Pushing and continuing loop.\n");
                push_stack(op_stack, *token); 
                continue; 
            }
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
                // printf("Popped operator from stack onto output stack.\n");
                push_stack(ret_stack, pop_stack(op_stack)); 
                op_stack_top = get_stack_top(op_stack); // otherwise an infinite loop results
                if (op_stack_top == NULL) { break; } // if stack just emptied, we'll get a null ptr
                // (and if we attempt to do stuff with that on the next run of the loop, bad stuff
                // is going to happen)
            }

            push_stack(op_stack, *token);
        } else if (token->type == Bracket_Left) {
            push_stack(op_stack, *token);
        } else if (token->type == Bracket_Right) {
            while (op_stack_top->type != Bracket_Left) {
                push_stack(ret_stack, pop_stack(op_stack));
                op_stack_top = get_stack_top(op_stack);
                if (op_stack_top == NULL) { break; }
            }
            // Once that loop is done, the operator stack will either be empty or have a left 
            // parentheses on top. If it's empty, that means there are mismatched parentheses.
            if (is_stack_empty(op_stack)) {
                return -1; // error return code (rc)
            } else if (op_stack_top->type == Bracket_Left) {
                pop_stack(op_stack); // Discard top
                //op_stack_top = get_stack_top(op_stack);
            }
        }

        // Update top stack pointer
        // printf("[Shunting] (i = %d) Finished evaluating token of type %d\n", i, token->type);
        // printf("[Shunting] Getting new stack top: Stack start (%p), Stack top (%p), ",
        //      op_stack->start, op_stack_top);
        op_stack_top = get_stack_top(op_stack);
        
        // if (op_stack_top == NULL) { break; } It's possible for the operator stack to be emptied
        // during the running of the program, I think. So it's mainly important to check for NULL
        // during loops when the stack top will *immediately* be accessed again, whereas here the
        // next iteration may add to it, so a break is unnecessary.

        // printf("New stack top (%p)\n", op_stack_top);
    }

    // Pop remainder of operator stack onto output queue
    // printf("Popping contents of operator stack into output stack...\n");
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

    // Cleanup
    delete_stack(op_stack);
    delete_stack(ret_stack);

    return j; // rc for success: number of tokens in the final rpn expression
}

// Safety net: sometimes once we pop the last element off the operator stack we will try and get
// the top of an empty stack, which generates a null pointer.
void refresh_op_stack_top(struct Token *stack_top_ptr, struct Stack *stack) {
    if (!is_stack_empty(stack)) {
        stack_top_ptr = get_stack_top(stack);
    }
}

// Evaluate RPN expression
double evaluate_rpn(struct Token *input_rpn, int num_tokens, double x) {
    // Variables used during iteration
    struct Token *token;
    struct Token operand1;
    struct Token operand2;
    double operand1_value;
    double operand2_value;
    double operation_result;
    
    // Initialize operand stack
    struct Stack *eval_stack = init_stack(num_tokens); 
    // In theory it doesn't need to be this big, but stack overflow errors are hard to debug due
    // to the website of the same name taking up all the Google results, so better to be safe ;)

    // Loop through all tokens (again, the arrays are backwards because they're generated by
    // popping stacks starting at the top)
    for (int i = num_tokens-1; i >= 0; i--) {
        token = (input_rpn + i);
        if (token == NULL) { break; }
        
        //printf("Token: ");
        //print_token(token);
        

        if (token->type == Operator) {
            // Get two most recent operands & their values
            // Their value depends on if they're a number or a variable
            operand1 = pop_stack(eval_stack);
            operand2 = pop_stack(eval_stack);
            

            operand1_value = get_token_value(&operand1, x);
            operand2_value = get_token_value(&operand2, x);

            // Now perform the calculation. Sorry again about this Great Wall of China replica
            switch (token->operator_type) {
                case Op_Power:
                    operation_result = pow(operand2_value, operand1_value);
                    break;
                case Op_Multiply:
                    operation_result = operand1_value * operand2_value;
                    break;
                case Op_Divide:
                    operation_result = operand1_value / operand2_value;
                    break;
                case Op_Add:
                    operation_result = operand1_value + operand2_value;
                    break;
                case Op_Subtract:
                    operation_result = operand1_value - operand2_value;
                    break;
                default:
                    printf("invalid operator???");
                    exit(EXIT_FAILURE);
            }

            //printf(" | Operation result: %f", operation_result);

            // Once we have the result, push it back to the stack
            struct Token result = {
                .type = Number,
                .value = operation_result
            };

            push_stack(eval_stack, result);
        }
        else if (token->type == Function) {
            // Pop the most recent value off the stack and use it as the paremeter to the function
            // I haven't implemented any functions which take more than one value, so this is valid
            // for all cases.
            operand1 = pop_stack(eval_stack);
            operand1_value = get_token_value(&operand1, x);

            switch (token->function_type) {
                case Func_Sin:
                    operation_result = sin(operand1_value);
                    break;
                case Func_Cos:
                    operation_result = cos(operand1_value);
                    break;
                case Func_Tan:
                    operation_result = tan(operand1_value);
                    break;
                case Func_Ln:
                    operation_result = log(operand1_value);
                    break;
                case Func_Log:
                    operation_result = log10(operand1_value);
                    break;
            }

            struct Token result = {
                .type = Number,
                .value = operation_result
            };

            push_stack(eval_stack, result);
        }
        else if (token->type == Number) {
            push_stack(eval_stack, *token);
        } 
        else if (token->type == Variable) {
            push_stack(eval_stack, *token);
        }

    }
    // Clean up
    // In theory once that loop is done, the result should be the lone value on the stack
    double result = pop_stack(eval_stack).value;
    delete_stack(eval_stack);

    return result;
}
