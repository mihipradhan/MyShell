#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main (int argc, char *argv[]){
    printf("argc: %d\n", argc);
    
    //print strings received from input
    printf("i have received many strings. here they are:\n"); 
    for (int i = 1; i < argc; i++){
        printf("string %d: %s\n", i + 1, argv[i]); 
    }
    return 0;
    //given();
    //return 17; 
}