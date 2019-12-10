#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stack.h"

/*
 * ----------------------------------------------
 * Function/type prototypes
 * ----------------------------------------------
 */

// User interface
int menu();
double get_double_input(const char*);

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

        if (choice == 2) {
            return EXIT_SUCCESS; // Quit program with appropriate exit code
        }

        // Else, continue on with the program
        // Next step: get input (expression, h, range)
        char expression[64]; // The expression to evaluate
        double h; // The width of the strips used in the approximation
        double start; // The lower value of the range
        double end; // The upper value of the range

        // Getting expression doesn't involve a function call, since it's just 2 lines and only one
        // string is ever taken from user input
        printf("Please enter an expression to perform integration of: ");
        fgets(expression, 65, stdin);

        h = get_double_input("Please enter a value for the strip width: ");
        start = get_double_input("Please enter the lower limit of integration: ");
        end = get_double_input("Please enter the upper limit of integration: ");

        

        // If they've been silly and start > end, swap them around
        if (start > end) {
            double tmp;
            tmp = start;
            start = end;
            end = tmp;
        }

        // next steps:
        // * convert expression to RPN using shunting yard
        // * in a loop, take RPN expression, replace x tokens with value of x used, evaluate
        // * perform above step as necessary to satisfy Simpson's rule/trapezium rule
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
    \t1. Compute trapezium rule estimate\n\
    \t2. Exit\n");

    char input;

    // Loop until valid input received
    while (1) {
        input = getchar();
        input -= 48; // 48 is the character 0 in ASCII; by subtracting this offset, input is an
                     // integer corresponding to the chosen option's number

        if (input >= 1 && input <= 2) { // valid range of choices
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
    printf(prompt);
    
    char buffer[255]; // Holder for string input
    char *n_end; // Pointer given to strtod which signifies the end of valid numerical input
    double output;

    // Loop until satisfactory input is received, at which point function returns said input
    while (1) {
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

// ------ Shunting yard / RPN-related functions ------

void FormatExpression(char *expression) {

}



