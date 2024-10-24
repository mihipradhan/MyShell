#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "arraylist.h"

#ifndef DEBUG
#define DEBUG 0
#endif

int count = 0; 

void al_init(arraylist_t *L, unsigned size){
    L->program = NULL; 
    L->data = malloc(size * sizeof(char*));
    L->redirectIn = 0; 
    L->redirectOut = 0;
    //L->args = 0; 
    L->redirectInFile = NULL; 
    L->redirectOutFile = NULL; 
    L->length = 0;
    L->capacity = size;
}

void al_destroy(arraylist_t *L){
    if (L->program != NULL){
        free(L->program); 
    }
    if (L->redirectInFile != NULL){
        free(L->redirectInFile);
    }
    if (L->redirectOutFile != NULL){
        free(L->redirectOutFile);
    }
    for (int i = 0; i < L->length; i++){
        free(L->data[i]); 
    }
    free(L->data);
}

unsigned al_length(arraylist_t *L){
    return L->length;
}

void al_giveArgsNum(arraylist_t *L, int item){
    L->args = item; 
}

int al_argsNum(arraylist_t *L){
    return L->args; 
}

int al_redirectIn(arraylist_t *L){
    return L->redirectIn; 
}

int al_redirectOut(arraylist_t *L){
    return L->redirectOut;
}

int al_inFile (arraylist_t *L, char **dest){
    if (L->redirectInFile == NULL){
        *dest = NULL;
        //printf("i'm returning null\n");
        return -1; 
    }
    *dest = L->redirectInFile; 
    //printf("i'm returning this %s\n", *dest);
    return 1;
}

int al_outFile (arraylist_t *L, char **dest){
    if (L->redirectOutFile == NULL){
        *dest = NULL; 
        //printf("i'm returning null\n");
        return -1; 
    }
    *dest = L->redirectOutFile; 
    //printf("i'm returning this %s\n", *dest);
    return 1; 
}

void al_addInFile(arraylist_t *L, char *item){
    //printf("item: %s\n", item);
    L->redirectInFile = item; 
    //printf("added into L: %s\n", L->redirectOutFile);
}

void al_addOutFile(arraylist_t *L, char *item){
    //printf("item: %s\n", item);
    L->redirectOutFile = item; 
   // printf("added into L: %s\n", L->redirectOutFile);
}

void al_pushprogram(arraylist_t *L, char *item){
    //printf("item: %s\n", item);
    L->program = item; 
    //printf("added into L: %s\n", L->program);
}

int al_popprogram(arraylist_t *L, char **dest){

    //printf("this is program: %s\n", L->program); 
    if (!L->program){
        return -1; 
    }
    *dest = L->program; 
    //printf("this is dest: %s\n", *dest); 
    return 1; 
}

void al_push(arraylist_t *L, char *item){
    if (L->length == L->capacity) {
        L->capacity *= 2;
        char **temp = realloc(L->data, L->capacity * sizeof(char*));
        if (!temp) {
            // for our own purposes, we can decide how to handle this error
            // for more general code, it would be better to indicate failure to our caller
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
        }
        L->data = temp;
        if (DEBUG) printf("Resized array to %u\n", L->capacity);
    }
    L->data[L->length] = item;
    L->length++;
}

// returns 1 on success and writes popped item to dest
// returns 0 on failure (list empty)
int al_pop(arraylist_t *L, char **dest){
    if (L->length == 0){
        count = 0; 
        return 0;
    } 
    L->length--;
    *dest = L->data[count];
    count++;
    if (L->length==0){
        count = 0; 
    }
    return 1;
}

char** al_arg(arraylist_t *L){
    if (L->length == 0){
        return NULL; 
    }
    char **dest = L->data; 
    return dest; 
}


