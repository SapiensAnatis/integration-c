#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 * --------------------------
 * Function: menu()
 * --------------------------
 * Description: Displays a list of choices to the user
 * Returns: Integer representing choice selected. Guaranteed to be either 1 or 2.
 */

int menu() {
    printf("Please select from the following options:\n\
    \t1. Compute trapezium rule estimate\n\
    \t2. Exit\n");

    char input;
    input = getchar();
    input -= 48; // 48 is 0 in ASCII; by subtracting this offset, input is an integer corresponding to the chosen option's number
    if (input >= 1 && input <= 2) { // valid range of choices
        return input;
    } else {
        printf("You have selected an invalid option. Please try again.\n");
        menu(); // Loop back and prompt again
    }
}

int main() {
    int choice;
    choice = menu();
}