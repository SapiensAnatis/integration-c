#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "tokenize.h"
#include "shunting.h"
#include "token.h"
#include "stack.h"
#include "project.h"

/*
 * ----------------------------------------------
 * Function definitions
 * ----------------------------------------------
 */

/*
 * Function: main()
 *
 * Description: Main subroutine of the program. Executed on startup
 * Parameters: none
 * Returns: Exit code, giving information about how the program performed (system dependant)
 */

int main() {
    while (1) {
        
        int choice;
        choice = menu();
        clear_stdin();

        if (choice == 4) {
            return EXIT_SUCCESS; // Quit program with appropriate exit code
        } else if (choice == 3) {
            // Show help
            printf(
"\nThis is an integral calculator using several different methods for numerically\n\
computing (i.e. computing without algebra) integral expressions. Mostly any\n\
expression is supported, aside from some more niche functions (hyperbolic trigs,\n\
binomial choose, to name a few) and similarly niche operators (e.g. factorial)\n\n\
A few points to note:\n\n\
\t* At this time, the only variable of integration supported is x. Use this,\n\
\t  and only this, when you want to use a variable.\n\
\t* Please always enclose function arguments in brackets: e.g. ln(x) instead of lnx.\n\
\t* The implemented functions are:\n\
\t\t- 'sin',\n\
\t\t- 'cos',\n\
\t\t- 'tan',\n\
\t\t- 'ln',\n\
\t\t- 'exp' (base e),\n\
\t\t- 'log' (which is base 10).\n\
\t* The above functions can be used in an expression by typing their name (as\n\
\t  enclosed in quotes above).\n\
\t* The implemented operators are:\n\
\t\t- Addition (+)\n\
\t\t- Subtraction (-)\n\
\t\t- Multiplication (*)\n\
\t\t- Division (/)\n\
\t\t- Exponents (^)\n\
\t* Implicit multiplication is supported (e.g. 4sin(x) will be interpreted as \n\
\t  4*sin(x))\n\n"
            );
            continue; // show menu again
        }

        // At this point, options 3/4 have broken the flow of the program, so we're only here
        // if we want to do some integration. We can therefore prepare for this, and only decide
        // which method to use later.1
        char expression[64] = ""; // The expression to evaluate
        int strips; // The width of the strips used in the approximation
        double start; // The lower value of the range
        double end; // The upper value of the range


        

        // Getting expression doesn't involve a function call, since it's just 2 lines and only one
        // string is ever taken from user input
        printf("\nPlease enter an expression to perform integration of: ");
        fgets(expression, 65, stdin); // includes newline from user, so 64+1
        // get rid of the newline
        int exp_length = strlen(expression);
        if (expression[exp_length-1] == '\n') { expression[exp_length-1] = 0; }

        // Tokenize expression
        // In considering the maximum tokens, it's tempting to say "the maximum is if each
        // character is a token, e.g. '2*3*4*5'", but this doesn't account for implicit 
        // multiplication - the maximum tokens is created by an expression like '()()()' which has
        // 6 characters but 8 tokens (even if that's a garbage expression, it's technically valid, 
        // it just produces an empty stack once shunting yard is done). So the maximum token:char 
        // ratio is not 1, but rather 8/6 = 4/3 ~= 1.333333

        // Strictly speaking, it *would* be more efficient to perform a regex match to detect
        // implicit multiplication and all tokens and allocate memory accordingly, but this current
        // solution uses less than 1MB for most simple expressions, and the expression is only
        // tokenized once, so I'm sure I won't be crashing any systems by letting laziness take
        // over in this case.

        int max_exp_tokens = (int)(ceil(strlen(expression) * 1.333333333));

        if (max_exp_tokens == 0) { // These checks exist to make sure we don't malloc() 0 bytes
            printf("\nIntegration result: 0\n\n"); // Don't even bother 
            continue;
        }
        
        struct Token* tokenized_exp = malloc(max_exp_tokens * sizeof(struct Token));
        int exp_tokens = exp_to_tokens(expression, tokenized_exp); // assigns the tokenized exp to 
                                                                   // that pointer (2nd argument)

        printf("\nTokenized: ");
        print_tokenized(tokenized_exp, exp_tokens);

        if (exp_tokens == 0) {
            printf("\nIntegration result: 0\n\n"); 
            continue;
        }

        struct Token* rpn_exp = malloc(exp_tokens * sizeof(struct Token)); // same deal
        int rc = shunting_yard(tokenized_exp, exp_tokens, rpn_exp); // rc: number of shunted tokens
        
        
        printf("RPN: ");
        print_tokenized(rpn_exp, rc);

        start = get_double_input("Please enter the lower limit of integration: ");
        end = get_double_input("Please enter the upper limit of integration: ");

        if (fabs(start-end) < 0.0000001) { 
            // Can't directly compare floats as they're weird
            // This is the next best thing to a == b
            printf("\nIntegration result: 0\n\n"); // Don't even bother 
            continue;
        }

        strips = get_int_input("Please enter the number of strips to use: ");        

        // Swap because integration method goes from lowest to highest
        if (start > end) {
            double tmp;
            tmp = start;
            start = end;
            end = tmp;
        }

        double dx = fabs(start - end);

        // printf("|%f - %f| = %f\n", start, end, dx);

        double h = dx / strips;
        double sum = 0;
        int four_or_two = 1;
        int n = 1;
        double y;
        // printf("h = %f\n", h);

        double current_x = start + h; // don't eval at start twice

        // --- Simpson's rule ---
        if (choice == 1) {
            // First add f(x_0) and f(x_n)
            sum += evaluate_rpn(rpn_exp, rc, start);
            sum += evaluate_rpn(rpn_exp, rc, end);

            while (current_x <= end) {
                if (n % 2 == 0) {
                    // if n is even
                    four_or_two = 2;
                } else {
                    // if n is odd
                    four_or_two = 4;
                }
                y = evaluate_rpn(rpn_exp, rc, current_x);
                // printf("x = %f, y = %f\n", current_x, y);
                sum += (four_or_two * y);
                
                n++;
                current_x += h;
            }

            // Finish by multiplying by dx/3
            sum *= h / 3;
        } 

        // --- Trapezium rule ---
        else if (choice == 2) {
            sum += evaluate_rpn(rpn_exp, rc, start);
            sum += evaluate_rpn(rpn_exp, rc, end);

            while (current_x <= end) {
                y = evaluate_rpn(rpn_exp, rc, current_x);
                sum += 2*y;
                current_x += h;
            }

            sum *= h / 2;
        }

        printf("\nIntegration result: %f\n\n", sum);


        // Avoid memory leaks, they aren't nice
        free(tokenized_exp);
        free(rpn_exp);
    }
}

// ------ User input functions ------

/*
 * Function: menu()
 *
 * Description: Displays a list of choices to the user
 * Parameters: none
 * Returns: Integer representing choice selected. Guaranteed to be either 1 or 2
 */

int menu() {
    printf("Please select from the following options:\n\
    \t1. Compute integration estimate by Simpson's rule\n\
    \t2. Compute integration estimate by trapezium rule\n\
    \t3. Show help message\n\
    \t4. Exit\n");

    char input;

    // Loop until valid input received
    while (1) {
        input = getchar();
        input -= 48; // 48 is the character 0 in ASCII; by subtracting this offset, input is an
                     // integer corresponding to the chosen option's number

        if (input >= 1 && input <= 4) { // valid range of choices
            return input;
        } else {
            printf("You have selected an invalid option. Please try again.\n");
            continue;
        }
    }
}

/*
 * Function: get_double_input(prompt)
 * 
 * Description: Displays a prompt to the user (as passed to the function) and interprets input as a 
 *              double type - with some error checking
 * Parameters: prompt - a character array (string) prompt which is given to printf() 
 *             to be shown to the user to inform their choice
 * Returns: double corresponding to interpretation (strtod). Will not return 0 unless the user 
 *          actually entered 0; invalid input is handled
 */

double get_double_input(const char* prompt) {
    char buffer[255]; // Holder for string input
    char *n_end; // Pointer given to strtod which signifies the end of valid numerical input
    double output;

    // Loop until satisfactory input is received, at which point function returns said input
    while (1) {
        printf(prompt);
        fgets(buffer, 256, stdin); // Assign stdin stream data to buffer; 256 not 255 because \n is
                                   // counted when the user presses Enter
        output = strtod(buffer, &n_end);
        
        if (n_end == buffer) { // If no numerical input was found
            printf("Please enter a valid number.\n");
            continue;
        } else {
            return output;
        }
    }
}

/*
 * Function: get_double_input(prompt)
 * 
 * Description: Displays a prompt to the user (as passed to the function) and interprets input as a 
 *              double type - with some error checking
 * Parameters: prompt - a character array (string) prompt which is given to printf() 
 *             to be shown to the user to inform their choice
 * Returns: double corresponding to interpretation (strtod). Will not return 0 unless the user 
 *          actually entered 0; invalid input is handled
 */

int get_int_input(const char* prompt) {
    char buffer[255]; // Holder for string input
    char *n_end; // Pointer given to strtod which signifies the end of valid numerical input
    int output;

    // Loop until satisfactory input is received, at which point function returns said input
    while (1) {
        printf(prompt);
        fgets(buffer, 256, stdin); // Assign stdin stream data to buffer; 256 not 255 because \n is
                                   // counted when the user presses Enter
        output = (int)strtol(buffer, &n_end, 10); // base10
        
        if (n_end == buffer || output == 0) { // If no numerical input was found
            printf("Please enter a valid number.\n");
            continue;
        } else {
            return output;
        }
    }
}

void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}