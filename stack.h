#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED // Include guards: block the same header from being included twice in a file
#include "token.h"

// Stacks
// --- Type declarations ---

struct Stack {
    struct Token *start; // Pointer to start of memory allocated for array of values
    struct Token *top; // Pointer to last defined value in array
    int size; // Current size of array
    int capacity; // Maximum size
};

// --- Function declarations ---
struct Stack *init_stack(int capacity);
void push_stack(struct Stack* stack, struct Token value);
struct Token pop_stack(struct Stack* stack);
struct Token *get_stack_top(struct Stack* stack);
struct Token* get_stack_start(struct Stack* stack);
int is_stack_empty(struct Stack* stack);
int get_stack_size(struct Stack* stack);
void delete_stack(struct Stack* stack);

#endif