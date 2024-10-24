#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[]){
    char str[100];
    //print strings received from input
    printf("int: %d\n", argc);
    for (int i = 1; i< argc; i++){
        printf("these are my strings: %s\n", argv[i]); 
    }
}