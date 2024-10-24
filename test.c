#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "arraylist.h"

int main(int argc, char *argv[]){
    arraylist_t A; 
    int args = 0; 
    al_init(&A, 4); 
    //al_pushprogram(&A, NULL);

    char buf [1000]; 

    int exitStat = 0; 

    int r = read(STDIN_FILENO, buf, 1000); 
    //char *string = (char*) malloc(sizeof(char) * r); 
    char *string; 
    char exitString[] = "exit"; 
    while ((exitStat !=1) && (r> 0)){
        string = (char*) malloc(sizeof(char) * r ); 
        memcpy(string, buf, r);
        string[r -1 ] = '\0'; 
        //printf("this is string: %s\n", string);
        if (strcmp(exitString, string) == 0){
            //printf("im here\n");
            exitStat = 1; 
            free(string);
            break; 
        }
        char *prog; 
        int num = al_popprogram(&A, &prog); 
        //printf("this is num: %d\n", num); 
        if (al_popprogram(&A, &prog) == -1){
            //printf("made it here\n");
            al_pushprogram(&A, string); 
        }
        else {
            al_push(&A, string); 
            args++; 
        }
        //al_push(&A, string); 
        r = read(STDIN_FILENO, buf, 1000); 
    }

    char *program; 
    int done = al_popprogram(&A, &program); 
    printf("this is program: %s\n", program); 
    free(program); 



    printf("list length: %u\n", al_length(&A)); 

    // char *n; 
    // while (al_pop(&A, &n)){
    //     printf("popped %s\n", n);
    //     free(n); 
    // }

    char **n; 
    n  = al_arg(&A); 
    //printf("something: %d\n", ); 
    for (int i = 0; i < args; i++){
        printf("%s\n", n[i]); 
        free(n[i]);
    }
    //ree(n); 

    //al_destroy(&A);
    // exitStat = 0; 
    // printf("im here\n");
    // r = read(STDIN_FILENO, buf, 1000);

    // while ((exitStat !=1) && (r> 0)){
    //     string = (char*) malloc(sizeof(char) * r ); 
    //     memcpy(string, buf, r);
    //     string[r -1 ] = '\0'; 
    //     printf("this is string: %s\n", string);
    //     if (strcmp(exitString, string) == 0){
    //         //printf("im here\n");
    //         exitStat = 1; 
    //         free(string);
    //         break; 
    //     }
    //     al_push(&A, string); 
    //     r = read(STDIN_FILENO, buf, 1000); 
    // }

    // while (al_pop(&A, &n)){
    //     printf("popped %s\n", n);
    //     free(n); 
    // }

    al_destroy(&A);

    return EXIT_SUCCESS;
}