#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h> // perl-compatible regexes, more modern
#include "stack.h"
#include "tokenize.h"
#include "shunting.h"

// ------ Shunting yard / RPN-related definitions ------

/*
 * ----------------------------------------------
 * Function definitions
 * ----------------------------------------------
 */


/* int main() {
    char expression[] = "4(sin(x))^2";
    double x = 4;
    int num_tokens;
    int max_tokens;

    max_tokens = (int)(ceil(strlen(expression) * 1.333333333));

    struct Token *tokenized = malloc(max_tokens * sizeof(struct Token));
    // printf("Allocated %d bytes for tokenized_exp array\n", max_tokens * sizeof(struct Token));
    num_tokens = exp_to_tokens(expression, tokenized);

    // printf("Input: %s\nOutput:", expression);
    print_tokenized(tokenized, num_tokens);
    // printf("Only needed %d bytes. Excess memory: %d bytes\n", 
    //  sizeof(struct Token) * num_tokens, 
    //  (max_tokens - num_tokens) * sizeof(struct Token)
    // );

    struct Token *rpn = malloc(num_tokens * sizeof(struct Token));
    int rc = shunting_yard(tokenized, num_tokens, rpn);
    printf("Shunting yard returned %d tokens. Shunted: ", rc);
    print_tokenized(rpn, rc);

    free(tokenized);

    // Evaluating expression at some value
    double result = evaluate_rpn(rpn, rc, x);

    printf("Given f(x) = %s, and x = %f\n", expression, x);
    printf("f(x) = %f", result);

    free(rpn);

    /*
     * Example input/output:
     * 
     * For the input expression "(x+1)(x+2)", the above code will print
     * 
     * ['(', 'x', '+', '1.00', ')', '*', '(', 'x', '+', '2.00', ')']
     * Shunting yard returned 7. Shunted: ['x', '1.00', '+', 'x', '2.00', '+', '*']
     */
//}


// Function: exp_to_tokens(expression)
// Description: Tokenizes expression, e.g. "3sin(0.1)" -> ["3", "sin", "(", "0.1", ")"]
// Parameters: expression, the string to be tokenized
// Outputs: A stack of all tokens

int exp_to_tokens(char *expression, struct Token *tokenized) {
    // Starting at the pointer for the string, run several regexes on the remainder of the string
    // until a token is recognized. Then move the pointer forward by the number of characters
    // in the recognized token, and repeat until the pointer points to \0
    
    // Initialize output
    // Same idea for calculating max number of tokens
    int max_tokens = (int)(ceil(strlen(expression) * 1.333333333));
    struct Stack *output = init_stack(max_tokens); 
    
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
        .precedence = 4,
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
    const struct Token x_var = {
        .type = Variable
    };

    #pragma endregion

    // Will also be helpful to keep track of the last token for filling in implicit multiplication
    // i.e. "if last token was a number and this token is a function" for things like 4sin(45)
    struct Token prev_token;
    // Initialize regex options
    int errornumber;
    int find_all = 0; // only want one match from regexes
    pcre2_match_data *match_data;
    int rc1;
    int rc2;
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
        //printf("Operating on substring %s\n", expression);
        switch(expression[0]) {
            case '(':
                // Implicit multiplication - if '4(' or ')(' is detected, substitute a * inbetween
                if (prev_token.type == Number || prev_token.type == Bracket_Right) { 
                    push_stack(output, multiply); 
                }

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
            case 'x':
                if (prev_token.type == Number || prev_token.type == Bracket_Right) {
                    push_stack(output, multiply); 
                }
                push_stack(output, x_var);
                prev_token = x_var;
                expression++;
                continue;
            case ' ': // no action required, move on
                expression++;
                continue;
            default: // break out to regex statements
                break;
        }
        
        // If none of those matched, then start using regex
        // (this can only be reached by avoiding the above continues)

        // Number regex
        subject = (PCRE2_SPTR)expression;
        match_data = pcre2_match_data_create_from_pattern(num_regex_comp, NULL);

        rc1 = pcre2_match(
            num_regex_comp,
            expression,
            strlen(expression),
            0,
            0,
            match_data,
            NULL
        );
        
        // If number regex matched
        if (rc1 > 0) {
            ovector = pcre2_get_ovector_pointer(match_data);
            //printf("Number match succeesed at offset %d\n", (int)ovector[0]);

            // Normally this is done in a loop for match groups, but not interested in those, only
            // the full match
            PCRE2_SPTR substring_start = subject + ovector[0];
            size_t substring_length = ovector[1] - ovector[0];
            
            // End disclaimer, this next bit I understand 
            // Make a space to copy the string into               
            char *substring = malloc(substring_length * sizeof(char));
            memcpy(substring, substring_start, substring_length);

            // Convert said string to a double
            double d = strtod(substring, NULL);
            // No longer need it, so
            free(substring);
            //printf("\tMatch found (length %d): %f\n", substring_length, d);

            struct Token num_token = {
                .type = Number,
                .value = d
            };

            push_stack(output, num_token);
            prev_token = num_token;
            expression += substring_length;
    
            continue;
        }

        match_data = pcre2_match_data_create_from_pattern(func_regex_comp, NULL);
        
        rc2 = pcre2_match(
            func_regex_comp,
            expression,
            strlen(expression),
            0,
            0,
            match_data,
            NULL
        );

        // If function regex matched
        if (rc2 > 0) {
            if (prev_token.type == Number || prev_token.type == Bracket_Right) {
                // Implicit multiplication again, checking for e.g. '4sin' or ')sin'
                push_stack(output, multiply);
            }
            ovector = pcre2_get_ovector_pointer(match_data);
            PCRE2_SPTR substring_start = subject + ovector[0];
            size_t substring_length = ovector[1] - ovector[0];

            char *substring = malloc(substring_length * sizeof(char));
            memcpy(substring, substring_start, substring_length);

            // Convert said string to a token
            // Bit messy. Would've preferred to use cases but you can't in C because Dennis Ritchie
            // clearly had something against second-year university students enjoying life when he 
            // decided to make strings arrays meaning they can't be used in switches
            enum Function_Type ft;

            // Lowercase the substring in case people choose to write things weirdly
            for (int i = 0; i < substring_length; i++) {
                substring[i] = tolower(substring[i]);
            }
            
            // printf("Found match for function regex of length %d\n", substring_length);

            if (strcmp(substring, "sin") == 0) { ft = Func_Sin; }
            else if (strcmp(substring, "cos") == 0) { ft = Func_Cos; }
            else if (strcmp(substring, "tan") == 0) { ft = Func_Tan; }
            else if (strcmp(substring, "ln") == 0) { ft = Func_Ln; }
            else if (strcmp(substring, "exp") == 0) { ft = Func_Exp; }
            else if (strcmp(substring, "log") == 0) { ft = Func_Log; }

            struct Token func_token = {
                .type = Function,
                .function_type = ft
            };

            push_stack(output, func_token);
            prev_token = func_token;
            expression += substring_length;

            free(substring);
            continue;
        }

        // Now, the only way you can be down here is if something went quite wrong

        if (rc1 == PCRE2_ERROR_NOMATCH && rc2 == PCRE2_ERROR_NOMATCH) { 
            printf("Unrecognized token found in input expression: '%c'\n", expression[0]);
            expression++;
            continue;
        }    
        else if (rc1 != PCRE2_ERROR_NOMATCH && rc2 != PCRE2_ERROR_NOMATCH) {
            printf("Something went wrong in either regex match attempt: rc1 = %d / rc2 = %d", 
            rc1, rc2);
        }

        pcre2_match_data_free(match_data);
    }

    // printf("Iteration complete. Popping stack of size %d\n", get_stack_size(output));
    // Take the output stack and pop it into an array at the address `tokenized`
    // Note that because this array is generated by taking the elements popped off the top of the
    // stack first, it's actually backwards compared to the input
    // The reason why we generate an array even though shunting yard, where it goes next, uses
    // stacks: it's not possible to iterate through the elements of my Stack type without modifying
    // it, which makes it impossible to debug the program :(
    int i = 0;
    // printf("i = ");
    while (!is_stack_empty(output)) {
        // printf("%d...", get_stack_size(output));
        // Write to the pointer given to the function (expected to be from malloc)
        struct Token value = pop_stack(output);
        memcpy(tokenized + i, &value, sizeof(value));
        i++;
    }

    // We're now done with the stack + others, so free the memory
    delete_stack(output);

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
