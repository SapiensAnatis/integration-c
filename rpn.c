#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h> // perl-compatible regexes, more modern
#include "rpn.h"

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

enum Function_Type {
    Func_Sin,
    Func_Cos,
    Func_Tan,
    Func_Ln,
    Func_Exp,
    Func_Log
};


enum Associativity {
    Assoc_Left,
    Assoc_Right
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

int main() {
    char *expression;
    int num_tokens;
    strcpy(expression, "2*4*5");
    struct Token *tokenized = malloc(6 * sizeof(struct Token));
    num_tokens = exp_to_tokens(expression, tokenized);
    for (int i = 0; i < num_tokens; i++) {
        switch(tokenized[i].type) {
            case Operator:
                printf("op ");
                break;
            default:
                break;
        }
    }
}


// Function: exp_to_tokens(expression)
// Description: Tokenizes expression, e.g. "3sin(0.1)" -> ["3", "sin", "(", "0.1", ")"]
// Parameters: expression, the string to be tokenized
// Outputs: A stack of all tokens

int exp_to_tokens(char *expression, struct Token *tokenized) {
    // Starting at the pointer for the string, run several regexes on the remainder of the string
    // until a token is recognized. Then move the pointer forward by the number of characters
    // in the recognized token, and repeat until the pointer points to \0
    
    // Initialize output
    struct Stack *output = init_stack(strlen(expression)); 
    
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

    // Will also be helpful to keep track of the last token for filling in implicit multiplication
    // i.e. "if last token was a number and this token is a function" for things like 4sin(45)
    struct Token prev_token;

    // Initialize regex options
    int errornumber;
    int find_all = 0; // only want one match from regexes
    pcre2_match_data *match_data;
    int rc;
    PCRE2_SPTR subject;
    PCRE2_SIZE *ovector;

    // Initialize all needed regexes outside of the loop for efficiency
    // Number token regex:
    // Match any number of digits, then, optionally, a decimal point followed by more digits
    pcre2_code *num_regex_comp;
    char *num_regex = "^\\d+(\\.\\d+)?"; 
    compile_regex(num_regex, &num_regex_comp);

    // Function token regex:
    // Match functions in a list (much easier than matching any 2-3 chars and checking if valid)
    pcre2_code *func_regex_comp;
    char *func_regex = "^(sin|cos|tan|ln|exp|log)";
    compile_regex(func_regex, &func_regex_comp);
    
    // The remaining possible tokens (brackets, operators, etc) are all one-character so don't need
    // their own regex

    // Loop of matching
    while (expression != NULL && expression[0] != '\0') {
        switch(expression[0]) {
            case '(':
                push_stack(output, bracket_l);
                prev_token = bracket_l;
                expression++; // Move forward 1 character
                continue; // Next iteration of the loop
            case ')':
                push_stack(output, bracket_r);
                prev_token = bracket_r;
                expression++;
                continue;
            case '^':
                push_stack(output, power);
                prev_token = power;
                expression++;
                continue;
            case '*':
                push_stack(output, multiply);
                prev_token = multiply;
                expression++;
                continue;
            case '/':
                push_stack(output, divide);
                prev_token = divide;
                expression++;
                continue;
            case '+':
                push_stack(output, add);
                prev_token = add;
                expression++;
                continue;
            case '-':
                push_stack(output, subtract);
                prev_token = subtract;
                expression++;
                continue;
            default:
                break;
        }
        
        // If none of those matched, then start using regex
        // (this can only be reached by avoiding the above continues)
        subject = (PCRE2_SPTR)expression;
        match_data = pcre2_match_data_create_from_pattern(num_regex_comp, NULL);
        rc = pcre2_match(
            num_regex_comp,
            expression,
            strlen(expression),
            0,
            0,
            match_data,
            NULL
        );
        if (rc > 0) {
            ovector = pcre2_get_ovector_pointer(match_data);
            printf("Number match succeesed at offset %d\n", (int)ovector[0]);
            for (int i = 0; i < rc; i++) {
                PCRE2_SPTR substring_start = subject + ovector[2*i];
                size_t substring_length = ovector[2*i+1] - ovector[2*i];
                
                char *substring = malloc(substring_length * sizeof(char));
                memcpy(substring, substring_start, substring_length);
                printf("Match found: %s\n", substring);
            }
            expression++;
            continue;
        }
        else if (rc == PCRE2_ERROR_NOMATCH) { // match failed
            printf("No match found for number regex");
        }    

        pcre2_match_data_free(match_data);
    }

    // Take the output stack and pop it into an array at the address `tokenized`
    int i = 0;
    while (!is_stack_empty(output)) {
        // Write to the pointer given to the function (expected to be from malloc)
        struct Token value = pop_stack(output);
        memcpy(tokenized + i, &value, sizeof(value));
        i++;
    }

    pcre2_code_free(num_regex_comp);
    pcre2_code_free(func_regex_comp);

    return i;
}

// Function: compile_regex
// Description: Compiles and optimizes (by way of pcre_study) a regex & handles any errors
// Parameters: regex_str, the string literal regular expression,
//             output, the pointer to the pcre object which the compilation result is saved to
// Outputs: None

void compile_regex(char *regex_str, pcre2_code **output) {
    int error_num;
    PCRE2_SIZE error_offset;
    
    PCRE2_SPTR pattern = (PCRE2_SPTR)regex_str;
    *output = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &error_num, &error_offset, NULL);

    if (output == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(error_num, buffer, sizeof(buffer));
        printf("Regex compilation error (offset = %d): could not compile regex '%s': %s\n",
            (int)error_offset, buffer);
        exit(EXIT_FAILURE);
    }
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
        struct Token t;
        printf("Stack is already empty! Value not popped. Returning null Token.");
        return t;
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