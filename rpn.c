#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pcre.h> // perl-compatible regexes, more modern
#define OVECCOUNT 30

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
    Bracket_Left, // having a separate enum for bracket and (left, right) seems a bit silly to me
    Bracket_Right,
    Function
};

enum Operator_Type {
    Op_Add,
    Op_Subtract,
    Op_Multiply,
    Op_Divide,
    Op_Power
};

enum Associativity {
    Assoc_Left,
    Assoc_Right
};

enum Function_Type {
    Func_Sin,
    Func_Cos,
    Func_Tan,
    Func_Ln,
    Func_Exp,
    Func_Log
};

// The reason why there are both Function and Operator fields, as well as Function_Type is so that
// when it comes time to evaluate the RPN expression, it is possible to both examine "is the token
// a function" and "what function is the token". If we only stored function types, to check the
// former would require `if token.Function_Type != null` which is less readable

struct Token {
    enum Token_Type type;
    // Number-exclusive property, but as Stack is monotype all tokens must have these properties
    // They will be null/uninitialized for token types that don't use them, though
    double value; 
    // Operator-exclusive properties
    enum Operator_Type operator_type;
    int precedence;
    enum Associativity associativity;
    // Function-exclusive properties
    enum Function_Type function_type;
};

/*
 * ----------------------------------------------
 * Function definitions
 * ----------------------------------------------
 */

// Function: exp_to_tokens(expression)
// Description: Tokenizes expression, e.g. "3sin(0.1)" -> ["3", "sin", "(", "0.1", ")"]
// Parameters: expression, the string to be tokenized
// Outputs: A stack of all tokens

struct Stack *exp_to_tokens(char *expression) {
    // Starting at the pointer for the string, run several regexes on the remainder of the string
    // until a token is recognized. Then move the pointer forward by the number of characters
    // in the recognized token, and repeat until the pointer points to \0

    struct Stack *output = init_stack(strlen(expression)); // Initialize output
    
    // Initialize all preset tokens
    // (i.e. all except numbers)
    // Could do this before the push, but then they'd be declared every time they're needed
    // This way is messier but it means they only get declared once
    // So, I'm sorry about this....but it's for 'the greater good'
    #pragma region Token_Defs

    const struct Token bracket_l = {
        .type = Bracket_Left,
    };
    const struct Token bracket_r = {
        .type = Bracket_Right,
    };
    const struct Token power = {
        .type = Operator,
        .operator_type = Op_Power,
        .precedence = 3,
        .associativity = Assoc_Right
    };
    const struct Token multiply = {
        .type = Operator,
        .operator_type = Op_Multiply,
        .precedence = 3,
        .associativity = Assoc_Left
    };
    const struct Token divide = {
        .type = Operator,
        .operator_type = Op_Divide,
        .precedence = 3,
        .associativity = Assoc_Left
    };
    const struct Token add = {
        .type = Operator,
        .operator_type = Op_Add,
        .precedence = 2,
        .associativity = Assoc_Left
    };
    const struct Token subtract = {
        .type = Operator,
        .operator_type = Op_Subtract,
        .precedence = 2,
        .associativity = Assoc_Left
    };
    const struct Token sin = {
        .type = Function,
        .function_type = Func_Sin
    };
    const struct Token cos = {
        .type = Function,
        .function_type = Func_Cos
    };
    const struct Token tan = {
        .type = Function, 
        .function_type = Func_Tan
    };
    const struct Token ln = {
        .type = Function,
        .function_type = Func_Ln
    };
    const struct Token log = {
        .type = Function,
        .function_type = Func_Log
    };
    const struct Token exp = {
        .type = Function,
        .function_type = Func_Exp
    };

    #pragma endregion

    // Initialize all needed regexes outside of the loop for efficiency

    // Number token regex:
    // Match any number of digits, then, optionally, a decimal point followed by more digits
    pcre *num_regex_comp;
    pcre_extra *num_regex_ex;
    char *num_regex = "^\\d+(\\.\\d+)?"; 
    compile_regex(num_regex, num_regex_comp, num_regex_ex);

    // Function token regex:
    // Match functions in a list (much easier than matching any 2-3 chars and checking if valid)
    pcre *func_regex_comp;
    pcre_extra *func_regex_ex;
    char *func_regex = "^(sin|cos|tan|ln|exp|log)";
    compile_regex(func_regex, func_regex_comp, func_regex_ex);
    
    // The remaining possible tokens (brackets, operators, etc) are all one-character so don't need
    // their own regex

    // Loop of matching
    while (expression != NULL && expression[0] != '\0') {
        if (expression[0] = '(') {
            push_stack(output, bracket_l);
            expression++;
        }
        else if (expression[0] = ')') {
            push_stack(output, bracket_r);
            expression++; // Move forward 1 character
        }
        else if (expression[0] = '^') {
            push_stack(output, power);
            expression++;
        }
        else if (expression[0] = '*') {
            push_stack(output, multiply);
            expression++;
        }
        else if (expression[0] = '/') {
            push_stack(output, divide);
            expression++;
        }
        else if (expression[0] = '+') {
            push_stack(output, add);
            expression++;
        }
        else if (expression[0] = '-') {
            push_stack(output, subtract);
        }
    }
}

// Function: compile_regex
// Description: Compiles and optimizes (by way of pcre_study) a regex & handles any errors
// Parameters: regex_str, the string literal regular expression,
//             output, the pointer to the pcre object which the compilation result is saved to
//             study_output, the pointer to the pcre_extra object which study output is saved to
// Outputs: None

void compile_regex(char *regex_str, pcre *output, pcre_extra *study_output) {
    char* error_str;
    int error_offset;

    output = pcre_compile(regex_str, 0, &error_str, &error_offset, NULL);
    if (output == NULL) {
        printf("Regex compilation error (offset = %d): could not compile regex '%s': %s\n",
        error_offset, regex_str, error_str);
        exit(EXIT_FAILURE);
    }

    study_output = pcre_study(output, 0, &error_str);
    if (error_str != NULL) {
        printf("Could not study regex '%s': %s\n",
        regex_str, error_str);
        exit(EXIT_FAILURE);
    }
}

// Function: generate_token(string_form)
// Description: Generates a Token struct from a string representation of a token
// Parameters: string_form, a string representation of a token - e.g. ")"
// Outputs: A token with the correct properties for the string

struct Token generate_token(char* string_form) {

}



// Function: scan_tokens(exp_substr)
// Description: Uses Regex to scan for any tokens that begin at the start of exp_substr
// Parameters: exp_substr, the string to scan - usually but not always a substring of an expression
// Outputs: a Token object representing a match found, or an uninitialized token if no match found


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

struct Stack *init_stack(int capacity) {
    struct Stack *s = malloc(sizeof(struct Stack));
    s->start = malloc(capacity * sizeof(struct Token)); // allocate a block of memory for the array
    
    if (s->start == NULL) {
        printf("Unable to allocate memory for stack! Please check that you have enough RAM free.");
        exit(EXIT_FAILURE);
    }

    // Once malloc has succeeded, can now assign the other properties
    s->capacity = capacity;
    s->top = s->start - 1; // First value should be at s.start, and push_stack assigns to s.top + 1
    s->size = 0;

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
    free(stack);
}