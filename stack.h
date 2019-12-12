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