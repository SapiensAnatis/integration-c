#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

// ------ Stack definitions ------
// The stack is a datatype that can be thought of like a stack of plates or books.
// You can add (push) items or remove (pop) them from the top, but you can't access arbitrary
// indices like you can with a normal array.
// A malloc() based approach is preferable to 'pretending' an array is a stack, because deleting
// or popping elements from traditional arrays is apparently quite difficult; this method gives 
// full access to the memory supporting the data structure, so deleting is as simple as moving
// the end pointer. Note that when this is done, the memory remains allocated in case it is needed
// for an item pushed at some later time.

/*
 * ----------------------------------------------
 * Struct definitions
 * ----------------------------------------------
 */

struct Stack {
    char *start; // Pointer to start of memory allocated for array of values
    char *top; // Pointer to last defined value in array
    int size; // Current size of array
    int capacity; // Maximum size
};

/*
 * ----------------------------------------------
 * Function definitions
 * ----------------------------------------------
 */


/*
 * Function: InitStack(capacity)
 *
 * Description: Factory method for creating and initializing stacks
 * Parameters: capacity - the maximum length of the stack
 * Returns: Empty stack with the capacity specified
 */

struct Stack InitStack(int capacity) {
    struct Stack s;
    s.start = malloc(capacity * sizeof(char)); // allocate a block of memory for the array
    
    if (s.start == NULL) { // malloc() returns null if it can't allocate, usually because of no RAM
        printf("Unable to allocate memory for stack! Please check that you have enough RAM free.");
        exit(EXIT_FAILURE); // Fatal error; program needs to quit
    }

    // Once malloc has succeeded, can now assign the other properties
    s.capacity = capacity;
    s.top = s.start - 1; // First value should be at s.start, and PushStack assigns to s.top + 1
    s.size = 0;

    return s;
}

/*
 * Function: PushStack(stack, value)
 *
 * Description: Method for adding items to the top of a stack
 * Parameters: stack, the stack to push to
 *             value, the value to push onto the stack
 * Returns: none
 */

void PushStack(struct Stack *stack, char value) {
    if (stack->size + 1 > stack->capacity) {
        printf("Stack full! Value not pushed.");
        return;
    }

    *(stack->top + 1) = value; // Write to the next address after the top of the stack
    stack->top++; // Increment the pointers to the top and the size value accordingly
    stack->size++;
}

/*
 * Function: PopStack(stack)
 *
 * Description: Deletes item from the top of a stack
 * Parameters: stack, the stack to pop from
 * Returns: The value that was popped from the stack
 */

char PopStack(struct Stack *stack) {
    if (stack->size == 0) {
        printf("Stack is already empty! Value not popped.");
        return;
    }
    
    char data;
    data = *(stack->top);
    
    stack->top--;
    stack->size--;
    
    printf("Popped: %c\n", data);
    return data;
}

/*
 * Function GetTopOfStack(stack)
 * 
 * Description: Gets the top element of the stack
 * Parameters: stack, the stack to get the top of
 * Returns: The element at the top of the stack given
 */

char GetTopOfStack(struct Stack *stack) {
    if (IsStackEmpty(stack)) {
        printf("Attempted to access top of empty stack!");
        return NULL;
    }

    return stack->top; 

    // These methods that simply return a property may not seem necessary, but I prefer that I have
    // them so that I never have to directly access the properties of the stack, since some of them
    // are memory addresses which could cause things to go wrong. It's much easier if direct access
    // is only performed inside a few controlled methods.
}

/*
 * Function IsStackEmpty(stack)
 * 
 * Description: Checks if the stack has any elements in it
 * Parameters: stack, the stack to examine
 * Returns: An integer - 0 = stack not empty / 1 = stack is empty
 */

int IsStackEmpty(struct Stack *stack) {
    return (stack->size == 0);
}

/*
 * Function DeleteStack(stack)
 * 
 * Description: Frees up the memory allocated by a stack
 * Parameters: stack, the stack to delete/free up
 * Returns: none
 */

void DeleteStack(struct Stack *stack) {
    free(stack->start);
}