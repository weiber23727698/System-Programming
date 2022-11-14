#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
    for(int i = 1;i <= 10;i++){
        int guess;
        /* initialize random seed: */
        srand ((atoi(argv[2]) + i) * 323);
        /* generate guess between 1 and 1000: */
        guess = rand() % 1001;
        printf("%d %d\n", atoi(argv[2]), guess);
    }
    return 0;
}