
#include <stdio.h>

int main() {   
    char buffer;
    printf("Napat limsuwan\n");    // Check if the program is running
    while(1) {
        scanf("%c", &buffer);      // Read a character from the keyboard
        if (buffer != 'q') {
            printf("%c", buffer);  // Print the character
        } else {
            break;                 // Exit the loop if the character is 'q'
        }
    }
    return 0;
}

/*
#include <stdio.h>

int main() {   
    char c;
    while(1) {
        scanf("%c", &c);     
        printf("%c\n", c); 
       }
    return 0;
}
*/
