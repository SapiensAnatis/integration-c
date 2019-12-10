#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

struct Stack;
void PushStack(struct Stack*, char);
char PopStack(struct Stack*);
void DeleteStack(struct Stack*);

#endif