#include <stdlib.h>
#include <stdio.h>

struct Stack {
    char *start; // Values held
    char *top; // Pointer to top value
    int size; // Current number of values held
    int capacity;  // Maximum number of values held
};

struct Stack InitStack(int capacity) {
    struct Stack s;
    s.start = malloc(capacity * sizeof(char));
    
    if (s.start == NULL) {
        printf("Out of memory!");
        exit(EXIT_FAILURE);
    }

    s.capacity = capacity;
    s.top = s.start - 1; // the first push should assign to the address s.start
    s.size = 0;

    return s;
}

void PushStack(struct Stack *stack, char value) {
    *((stack->top) + 1) = value;
    stack->top++;
    stack->size++;
}

char PopStack(struct Stack *stack) {
    char data;
    data = *(stack->top);
    
    stack->top--;
    stack->size--;
    
    printf("Popped: %c\n", data);
    return data;
}


int main() {
    struct Stack myStack;
    myStack = InitStack(4);
    
    PushStack(&myStack, 'a');
    printf("%p = %c\n", myStack.top, *myStack.top);
    PushStack(&myStack, 'b');
    printf("%p = %c\n", myStack.top, *myStack.top);
    PushStack(&myStack, 'c');
    printf("%p = %c\n", myStack.top, *myStack.top);
    PushStack(&myStack, 'd');
    printf("%p = %c\n", myStack.top, *myStack.top);

}