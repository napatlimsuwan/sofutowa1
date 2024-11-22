#include <stdio.h>

int main() {   
    char buf;
    printf("\033[2J\033[0;0H");
    printf("Hello World!\n");
    while(1) {
        scanf("%c", &buf);
        // *(char *)0x00d00039 = buf;
        if (buf != 'q') {
            printf("%c", buf);
        } else {
            break;
        }
    }
    printf("Hello World!!\n");
    return 0;
}