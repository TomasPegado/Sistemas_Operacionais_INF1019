#include <stdio.h>

int main(int argc, char *argv[]) {
    // Check if there are any command line arguments
    if (argc > 1) {
        printf("Arguments passed to the program:\n");
        // Loop through each argument and print it
        for (int i = 1; i < argc; i++) {
            printf("Argument %d: %s\n", i, argv[i]);
        }
    } else {
        // Print this if no arguments were passed
        printf("No arguments were passed to the program.\n");
    }

    return 0;
}
