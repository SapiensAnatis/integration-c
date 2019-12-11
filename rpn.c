#include <stdlib.h>
#include <stdio.h>

// ------ Shunting yard / RPN-related definitions ------

/*
 * ----------------------------------------------
 * Type definitions
 * ----------------------------------------------
 */

enum Token_Type {
    Operator, 
    Number,
    Variable,
    Bracket,
    Function
};

enum Associativity {
    Left,
    Right
};

struct Token {
    char text[16]; // if you want to use numbers with >15 digits then use someone else's program
    enum Token_Type type;
    // Operator-exclusive properties, but as Stack is monotype they're in the general Token struct
    int precedence;
    enum Associativity associativity;
};

/*
 * ----------------------------------------------
 * Function definitions
 * ----------------------------------------------
 */

// Function: exp_to_tokens(expression, length)
// Description: Tokenizes expression, e.g. "3sin(0.1)" -> ["3", "sin", "(", "0.1", ")"]
// Parameters: expression, the string to be stripped
//             array_ptr, the return value of malloc(strlen(expression) * sizeof(struct Token))
// Outputs: Pointer to a stack of all tokens

struct Stack* exp_to_tokens(char *expression) {
    
}

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
 * Type definitions
 * ----------------------------------------------
 */

struct Stack {
    struct Token *start; // Pointer to start of memory allocated for array of values
    struct Token *top; // Pointer to last defined value in array
    int size; // Current size of array
    int capacity; // Maximum size
};

/*
 * ----------------------------------------------
 * Function definitions
 * ----------------------------------------------
 */


/*
 * Function: init_stack(capacity)
 *
 * Description: Factory method for creating and initializing stacks
 * Parameters: capacity - the maximum length of the stack
 * Returns: Empty stack with the capacity specified
 */

struct Stack init_stack(int capacity) {
    struct Stack s;
    s.start = malloc(capacity * sizeof(struct Token)); // allocate a block of memory for the array
    
    if (s.start == NULL) {
        printf("Unable to allocate memory for stack! Please check that you have enough RAM free.");
        exit(EXIT_FAILURE);
    }

    // Once malloc has succeeded, can now assign the other properties
    s.capacity = capacity;
    s.top = s.start - 1; // First value should be at s.start, and push_stack assigns to s.top + 1
    s.size = 0;

    return s;
}

/*
 * Function: push_stack(stack, value)
 *
 * Description: Method for adding items to the top of a stack
 * Parameters: stack, the stack to push to
 *             value, the value to push onto the stack
 * Returns: none
 */

void push_stack(struct Stack *stack, struct Token value) {
    if (stack->size + 1 > stack->capacity) {
        printf("Stack full! Value not pushed.");
        return;
    }

    *(stack->top + 1) = value; // Write to the next address after the top of the stack
                               
    stack->top++; // Increment the pointers to the top and the size value accordingly
    stack->size++;
    // Due to the way pointer arithmatic is defined, +1 or ++ will actually
    // increment by sizeof(struct Token)
}

/*
 * Function: pop_stack(stack)
 *
 * Description: Deletes item from the top of a stack
 * Parameters: stack, the stack to pop from
 * Returns: The value that was popped from the stack
 */

struct Token pop_stack(struct Stack *stack) {
    if (stack->size == 0) {
        printf("Stack is already empty! Value not popped.");
        return;
    }
    
    struct Token data;
    data = *(stack->top);
    
    stack->top--;
    stack->size--;
    
    printf("Popped: %c\n", data);
    return data;
}

/*
 * Function get_stack_top(stack)
 * 
 * Description: Gets the top element of the stack
 * Parameters: stack, the stack to get the top of
 * Returns: The element at the top of the stack given
 */

struct Token* get_stack_top(struct Stack *stack) {
    if (is_stack_empty(stack)) {
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
 * Function is_stack_empty(stack)
 * 
 * Description: Checks if the stack has any elements in it
 * Parameters: stack, the stack to examine
 * Returns: An integer - 0 = stack not empty / 1 = stack is empty
 */

int is_stack_empty(struct Stack *stack) {
    return (stack->size == 0);
}

/*
 * Function delete_stack(stack)
 * 
 * Description: Frees up the memory allocated by a stack
 * Parameters: stack, the stack to delete/free up
 * Returns: none
 */

void delete_stack(struct Stack *stack) {
    free(stack->start);
}