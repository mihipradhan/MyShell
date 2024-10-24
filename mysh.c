#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <glob.h>


#include "arraylist.h"


#define BUFSIZE 1000

int interactiveMode = 0; 
int exitStat = 0; 
int pipeGiven = 0;
int isInFile = 0; 
int isOutFile = 0;
int lastComExit = 0; 
int conditional = -1; 
char *shellName; 
arraylist_t A; 
arraylist_t B; 

int conditionalCom (char *token){
    char stringThen[] = "then"; 
    char stringElse[] = "else"; 
    int val = -1; 

    if (strcmp(token, stringThen) == 0){
        val = 0; 
        return val; 
    }

    else if (strcmp(token, stringElse) == 0){
        val = 1; 
        return val; 
    }

    else {
        return -1; 
    }
}


int containsAsterisk(char *command){
    int yes = -1; 
    //printf("i be here\n"); 
    for (int i = 0; i < strlen(command); i++){
        if (command[i] == '*'){
            yes = i; 
            //printf("here\n"); 
            break; 
        }
    }
    return yes; 
}

void stepThroughCom (arraylist_t *L, char *command){
    char *beginToken = command; 
    int beginIndex = 0; 
    int tokenSize = 0;  
    int args; 
    int redirectIn = 0; 
    int redirectOut = 0; 

    //check first token for conditionals
    if (strlen(command) >= 4){
        char *tok = (char *) malloc (sizeof(char) * 5); 
        memcpy(tok, beginToken, 4); 
        tok[4] = '\0'; 
        int cond = conditionalCom(tok); 
        if (cond != -1){
            conditional = cond; 
            command = command + 5;
            beginToken = command; 
        }
        free(tok); 

    }

    
    //go through buffer and separate each token
    for (int i = 0; i < strlen(command); i++){

        //deal with the token at the end of the string
        if (i == strlen(command) -1){
            tokenSize = i  - beginIndex + 1; 
            char *newToken =  (char *) malloc (sizeof(char) * tokenSize +1); 
            memcpy(newToken, beginToken, tokenSize); 
            newToken[tokenSize] = '\0';
            //printf("hi\n"); 
            int asterisk = containsAsterisk(newToken); 
            //printf("asterisk: %d\n", asterisk); 
            if (isInFile != 0){
                al_addInFile(L, newToken); 
                isInFile = 0; 
            }
            else if (isOutFile != 0){
                al_addOutFile(L, newToken); 
                isOutFile = 0; 
            }
            else{
                char *prog;
                if (al_popprogram(L, &prog) == -1){
                    al_pushprogram(L, newToken);
                }
                else{
                    if (asterisk != -1){
                        glob_t blob = {0}; 
                        glob (newToken, GLOB_DOOFFS, 0, &blob); 
                        if (blob.gl_pathc != 0){
                            for (size_t i = 0; i < blob.gl_pathc; i++){
                                al_push (L, blob.gl_pathv[i]); 
                            }
                            free(newToken); 
                        }
                        
                        else {
                           al_push(L, newToken); 
                        }
                        //globfree(&blob); 
                    }
                    else {
                        al_push(L, newToken);
                    }
                }
            }
        }
        if (command[i] == ' '){
            tokenSize = i - beginIndex; 
            char *newToken =  (char *) malloc (sizeof(char) * tokenSize +1); 
            memcpy(newToken, beginToken, tokenSize); 
            newToken[tokenSize] = '\0';
            beginIndex = i + 1; 
            beginToken = beginToken + tokenSize + 1; 
            //printf("here !\n"); 
            int asterisk = containsAsterisk(newToken); 
            //printf("asterisk: %d\n", asterisk); 
            if (isInFile != 0){
                al_addInFile(L, newToken); 
                isInFile = 0;
            }
            else if (isOutFile != 0){
                al_addOutFile(L, newToken); 
                isOutFile = 0;
            }
            else{
                char *prog;
                if (al_popprogram(L, &prog) == -1){
                    al_pushprogram(L, newToken);
                }
                else{
                    if (asterisk != -1){
                        glob_t blob = {0}; 
                        glob (newToken, GLOB_DOOFFS, 0, &blob); 
                        if (blob.gl_pathc != 0){
                            for (size_t i = 0; i < blob.gl_pathc; i++){
                                al_push (L, blob.gl_pathv[i]); 
                            }
                            free(newToken); 
                        }
                        else {
                           al_push(L, newToken); 
                        }
                        //globfree(&blob);
                    }
                    else {
                        al_push(L, newToken);
                    }
                }
            }
        }
        if (command[i] == '|'){
            pipeGiven = 1; 
            //check to see if the previous char is whitespace; if not, we must add previous token
            if (command[i-1] != ' '){
                tokenSize = i - beginIndex; 
                char *newToken =  (char *) malloc (sizeof(char) * tokenSize +1); 
                memcpy(newToken, beginToken, tokenSize); 
                newToken[tokenSize] = '\0';
                beginIndex = i + 1; 
                beginToken = beginToken + tokenSize; 
                int asterisk = containsAsterisk(newToken); 
                char *prog;
                if (al_popprogram(L, &prog) == -1){
                    al_pushprogram(L, newToken);
                }
                else{
                    if (asterisk != -1){
                        glob_t blob = {0}; 
                        glob (newToken, GLOB_DOOFFS, 0, &blob); 
                        if (blob.gl_pathc != 0){
                            for (size_t i = 0; i < blob.gl_pathc; i++){
                                al_push (L, blob.gl_pathv[i]); 
                            }
                            free(newToken); 
                        }
                        else {
                           al_push(L, newToken); 
                        }
                        //globfree(&blob);
                    }
                    else {
                        al_push(L, newToken);
                    }
                }
            }

            // place beginToken and beginIndex  at the first char of the next token
            if (command[i + 1] == ' '){
                beginIndex = i + 2; 
                beginToken = beginToken + 2;  
                i = beginIndex; 
            }
            else {
                beginIndex = i + 1; 
                beginToken = beginToken + 1; 
            }

            stepThroughCom(&B, beginToken); 
            i = strlen(command);
            
        }
        if (command[i] == '>'){
            // mark that there is a redirection file provided and the next arg should not be added to the args arraylist
            isOutFile = 1; 
            //check to see if the previous char is whitespace; if not, we must add previous token 
            if (command[i-1] != ' '){
                tokenSize = i - beginIndex; 
                char *newToken =  (char *) malloc (sizeof(char) * tokenSize +1); 
                memcpy(newToken, beginToken, tokenSize); 
                newToken[tokenSize] = '\0';
                beginIndex = i + 1; 
                beginToken = beginToken + tokenSize; 
                int asterisk = containsAsterisk(newToken); 
                char *prog; 
                if (al_popprogram(L, &prog) == -1){
                    al_pushprogram(L, newToken);
                }
                else{
                    if (asterisk != -1){
                        glob_t blob = {0}; 
                        glob (newToken, GLOB_DOOFFS, 0, &blob); 
                        if (blob.gl_pathc != 0){
                            for (size_t i = 0; i < blob.gl_pathc; i++){
                                al_push (L, blob.gl_pathv[i]); 
                            }
                            free(newToken); 
                        }
                        else {
                           al_push(L, newToken); 
                        }
                        //globfree(&blob);
                    }
                    else {
                        al_push(L, newToken);
                    }
                }
            }
            // place beginToken and beginIndex  at the first char of the next token
            if (command[i + 1] == ' '){
                beginIndex = i + 2; 
                beginToken = beginToken + 2; 
                i = beginIndex; 
            }
            else {
                beginIndex = i + 1; 
                beginToken = beginToken + 1; 
            }

        }
        
        if (command[i] == '<'){
            // mark that there is a redirection file provided and the next arg should not be added to the arraylist
            redirectIn = 1;
            isInFile = 1; 
            if (command[i-1] != ' '){
                tokenSize = i - beginIndex; 
                char *newToken =  (char *) malloc (sizeof(char) * tokenSize +1); 
                memcpy(newToken, beginToken, tokenSize); 
                newToken[tokenSize] = '\0';
                beginIndex = i + 1; 
                beginToken = beginToken + tokenSize; 
                int asterisk = containsAsterisk(newToken); 
                char *prog;
                if (al_popprogram(L, &prog) == -1){
                    al_pushprogram(L, newToken);
                }
                else{
                    if (asterisk != -1){
                        glob_t blob = {0}; 
                        glob (newToken, GLOB_DOOFFS, 0, &blob); 
                        if (blob.gl_pathc != 0){
                            for (size_t i = 0; i < blob.gl_pathc; i++){
                                al_push (L, blob.gl_pathv[i]); 
                            }
                            free(newToken); 
                        }
                        else {
                           al_push(L, newToken); 
                        }
                        //globfree(&blob);
                    }
                    else {
                        al_push(L, newToken);
                    }
                }
            }

            if (command[i + 1] == ' '){
                beginIndex = i + 2; 
                beginToken = beginToken + 2; 
                i = beginIndex; 
            }
            else {
                beginIndex = i + 1; 
                beginToken = beginToken + 1; 
            }

        }
    }

    //free(command); 

    
}

void changeArgs(arraylist_t *L, int fileDescriptor){


    char *buffer = (char *) malloc (sizeof(char) *1000); 
    for (int i = 0; i < BUFSIZE; i++){
        buffer[i] = '\0';
    }
    char *partialCommand;  
    int beginWord; 
    int wordSize; 
    char *beginPtr; 
    int needAppend = 0; 
    int partialComWordSize; 
    char *command; 
    int count = 1; 
    int partialInitialized = 0;

    //fill buffer
    int reading = read(fileDescriptor, buffer, 1000);

    while (reading > 0 && exitStat != -1){

        beginWord = 0; 
        wordSize = 0; 
        beginPtr = buffer;  

        //go through buffer
        for (int i = 0; i < reading; i++){

            //if we've reached the end of buffer and the last char is not a new line, create ptr for partial command
            if ((i == reading-1) && (buffer[i] != '\n')){

                if (needAppend == 1){
                    fflush(stdout);
                    wordSize = i - beginWord + 1; 
                    command = (char *) malloc ((sizeof(char) * (wordSize + partialComWordSize + 1)));
                    memcpy(command, partialCommand, partialComWordSize); 
                    memcpy(command + partialComWordSize, beginPtr, wordSize); 
                    command[wordSize + partialComWordSize] = '\0'; 
                    needAppend = 0; 
                    for (int i = 0; i < strlen(partialCommand); i++){
                        partialCommand[i] = '\0';
                    }
                    free(partialCommand);
                    partialCommand = (char *) malloc ((sizeof(char) * (wordSize + partialComWordSize + 1))); 
                    partialInitialized = 1; 
                    memcpy(partialCommand, command, strlen(command));
                    partialCommand[strlen(command) + 1] = '\0'; 
                    free(command);
                }
                else{
                    wordSize = i - beginWord + 1; 
                    partialCommand = (char *) malloc(sizeof(char) *wordSize + 1); 
                    partialInitialized = 1; 
                    memcpy (partialCommand, beginPtr, wordSize); 
                    partialCommand[wordSize] = '\0';
                    partialComWordSize = wordSize; 
                    needAppend = 1;
                }
            }

            //if we've reached the newline, meaning end of command
            if (buffer[i] == '\n'){
                
                //if we have a partially completed command from last buffer fill, append
                if (needAppend ==1){
                    wordSize = i - beginWord; 
                    command = (char *) malloc(sizeof(char) * (wordSize + partialComWordSize + 1));
                    memcpy(command, partialCommand, partialComWordSize); 
                    memcpy (command + partialComWordSize, beginPtr, wordSize); 
                    command[wordSize + partialComWordSize] = '\0'; 
                    beginPtr = beginPtr + wordSize + 1; 
                    beginWord = i + 1; 
                    needAppend = 0; 
                    free(partialCommand);
                }

                else {
                    fflush(stdout);
                    wordSize = i - beginWord; 
                    command = (char *) malloc (sizeof(char) * wordSize +1); 
                    memcpy(command, beginPtr, wordSize); 
                    command[wordSize] = '\0'; 
                    beginPtr = beginPtr + wordSize +1; 
                    beginWord = i + 1; 
                }
        
                stepThroughCom(L, command); 
                free(command); 
            }
            
        }
        
        //clear buffer
        for (int i  = 0; i < BUFSIZE; i++){
            buffer[i] = '\0';
        }
        
        //fill buffer again
        reading = read(fileDescriptor, buffer, BUFSIZE);
 
    }


    //check to see if there's a partial command waiting to be finished 
    if (partialInitialized == 1){
    if (partialCommand[0] != '\0'){
        if (interactiveMode == 1){
            fflush(stdout); 
        }
        stepThroughCom(L, partialCommand); 
        // for (int i = 0; i < strlen(partialCommand); i++){
        //         partialCommand[i] = '\0';
        //     } 
        free(partialCommand);
        
    }
    }

    free(buffer); 

    
}


int executeCd(arraylist_t *L, int openIn, int openOut){
    pid_t child = fork();

    //enter child process
    if (child == 0){
        //redirect output files if needecd

        if (openOut == 1){
            //do nothing
        }
        else{
            dup2(openOut, STDOUT_FILENO);
        }

        if (openIn == 1){
            //do nothing
        }
        else{
            dup2(openIn, STDIN_FILENO);
        }

        int args = al_length(L); 
        if (args != 1){
            printf("wrong number of args\n"); 
            exit(0); 
        }

        char *n; 
        al_pop(L, &n); 

        int changeDir = chdir(n); 
        if (changeDir == -1){
            printf("could not change dir\n"); 
            exit(0);
        }

        // if (openIn != 0){
        //     close(openIn); 
        // }
        // if (openOut != 1){
        //     close(openOut);
        // }
        
        free(n); 
        al_destroy(L);
        exit(0);

    }

    else{
        //wait for child to finish 
        wait(NULL); 


        if (openOut == -1){
            return -1; 
        }
        if (openIn == -1){
            return -1;
        }

        int args = al_length(L); 
        if (args != 1){
            return -1; 
        }

        char *n; 
        al_pop(L, &n); 

        int changeDir = chdir(n); 
        if (changeDir == -1){
            return -1; 
        }

        else{
            shellName = n + strlen(n) -1; 
            for (int i = strlen(n) -1; i >0; i--){
                if (n[i-1] != '/'){
                    shellName = shellName -1; 
                }
                else{
                    break;
                }
            }

        }

    }

    return 0; 

}

int executePwd(arraylist_t *L, int openIn, int openOut){
    int returnVal = 0; 
    pid_t child = fork(); 

    //enter child process 
    if (child == 0){
        char buffer[500];
        for (int i = 0; i < 500; i++){
            buffer[i] = '\0';
        } 

        //redirect files if needed
        if (openOut == 1){
            //do nothing
        }
        else{
            dup2(openOut, STDOUT_FILENO);
        }

        if (openIn == 1){
            //do nothing
        }
        else{
            dup2(openIn, STDIN_FILENO);
        }

        //get working directory and print if sucess, indicate failure if not 
        char *workingDir = getcwd(buffer, 500); 
        if (workingDir == NULL){
            printf("could not get current working directory\n"); 
        }
        else{
            printf("%s\n", workingDir); 
        }

        al_destroy(L); 
        exit(0); 

    }
    //enter parent
    else{
        //wait for child to finish
        wait(NULL); 
        char buffer[500]; 
        for (int i = 0; i < 500; i++){
            buffer[i] = '\0';
        } 

        //if file could not be opened, return failure
        if (openOut == -1){
            returnVal = -1; 
        }

        //return failure if getcwd fails or sucess if it doesn't
        char *workingDir = getcwd(buffer, 500); 
        if (workingDir == NULL){
            returnVal =-1; 
            
        }
        else{
            returnVal = 0; 
        }
        return returnVal; 

    }
   
}

int executeExit(arraylist_t *L, int openIn, int openOut){

    pid_t child = fork(); 

    //enter child
    if (child == 0){

        //redirect files if needed 
        if (openOut == 1){
            //do nothing
        }
        else{
            dup2(openOut, STDOUT_FILENO);
        }

        if (openIn == 1){
            //do nothing
        }
        else{
            dup2(openIn, STDIN_FILENO);
        }
        
        //print arguments given if there are any
        char **n;
        if (al_length(L) > 0){
            n = al_arg(L); 
            for (int i = 0; i < al_length(L); i++){
                printf("%s\n", n[i]);
            }
        }

        al_destroy(L); 
        exit(0); 
    }
    else{
        wait(NULL); 

        //if unable to open redirection out file set lastComExit to 1 to indicate failure
        if (openOut == -1){
            return -1; 
        }
        else {
            exitStat = 1; 
            return 0; 
        }

    }

}

int executePath (arraylist_t *L, char *path, int newIn, int newOut){

    pid_t child = fork(); 

    if (child == 0){
        //redirect output files if needecd
        if (newOut == 1){
            //do nothing
        }
        else{
            dup2(newOut, STDOUT_FILENO);
        }

        //redirect input files if needed
        if (newIn == 0){
           //do nothing
        }
        else{
            dup2(newIn, STDIN_FILENO);
        }

        char *nullPtr = path; 
        char **n = al_arg(L); 
        char **new = malloc (sizeof (char *) * (al_length(L) + 1)); 
        new[0] = nullPtr; 
        for (int i = 1; i< al_length(L) + 1; i++){
            new[i] = n[i-1];
            
        }
        int i = execv(path, new);
        al_destroy(L); 
        exit(0); 
    }

    else{

        wait(NULL);
        return 0; 

    }

    
}

int executeBareName(arraylist_t *L, char *name, int openIn, int openOut){
    int returnVal = 0; 
    char *n = (char *) malloc (sizeof(char) * (15+ strlen(name) + 1)); 
    char directoryUsr[] = "/usr/local/bin/";  
    strcpy(n, directoryUsr); 
    strcat(n, name); 
    int exists = access(n, F_OK); 

    if (exists == 0){
        returnVal = executePath(L, n, openIn, openOut); 
        free(n); 
        return returnVal; 
    }
    else {
        free(n); 
        char *m = (char *) malloc (sizeof(char) * (15+ strlen(name) + 1)); 
        char directoryUsrBin[] = "/usr/bin/"; 
        strcpy(m, directoryUsrBin); 
        strcat(m, name); 
        exists = access(m, F_OK); 

        if (exists == 0){
            returnVal = executePath(L, m, openIn, openOut);  
            free(m); 
            return returnVal; 
        }
        else {
            free(m); 
            char *a = (char *) malloc(sizeof(char) * (5 + strlen(name) + 1));
            char directoryBin[] = "/bin/";  
            strcpy(a, directoryBin); 
            strcat(a, name); 
            exists = access(a, F_OK); 

            if (exists == 0){ 
                returnVal = executePath(L, a, openIn, openOut); 
                free(a);
                return returnVal; 
            }
            else {
                return -1; 
            }
            
        }
        
    }
}
char *pathName (arraylist_t *L){
    char *returnChar = NULL; 
    char **n = al_arg(L); 
    char *new = (char *) malloc (sizeof(char) * (15 + strlen(n[0]) + 1)); 
    char directoryUsr[] = "/usr/local/bin/"; 
    strcpy(new, directoryUsr); 
    strcat(new, n[0]); 
    int exists = access(new, F_OK); 
    int returnVal; 

    if (exists == 0){
        returnChar = new; 
        return returnChar; 
    }
    else {
        free (new); 
        char *m = (char *) malloc (sizeof(char) * (9+ strlen(n[0]) + 1)); 
        char directoryUsrBin[] = "/usr/bin/"; 
        strcpy(m, directoryUsrBin);
        strcat(m, n[0]); 
        exists = access(m, F_OK); 

        if (exists == 0){
            returnChar = m; 
            return returnChar; 
        }

        else {
            free(m); 
            char *a = (char *) malloc (sizeof(char) * (5+ strlen(n[0]) + 1)); 
            char directoryUsrBin[] = "/bin/"; 
            strcpy(a, directoryUsrBin);
            strcat(a, n[0]); 
            exists = access(a, F_OK); 

            if (exists == 0){
                returnChar = a;  
                return returnChar; 
            }
            else {
                free(a); 
                return returnChar; 
            }
        }

    }   
}

int executeWhich (arraylist_t *L, char *program, int openIn, int openOut){

    char *fileName  = pathName(L);
    
    pid_t child = fork(); 
    if (child == 0){

        if (openOut == 1){
            //do nothing
        }
        else{
            dup2(openOut, STDOUT_FILENO);
        }

        if (openIn == 0){
            //do nothing
        }
        else{
            dup2(openIn, STDIN_FILENO);
        }
        
        if (fileName != NULL){
            printf("%s\n", fileName); 
        }
        else {
            printf("not found\n"); 
        }

        // if (openIn != 0){
        //     close(openIn); 
        // }
        // if (openOut != 1){
        //     close(openOut);
        // }

        free(fileName); 
        exit(0); 
    }
    else{
        wait(NULL); 
        //free(fileName); 
        if (fileName != NULL){
            free(fileName); 
            return 0; 
        }
        else {
            free(fileName); 
            return -1; 
        }
    }

}

int executeCommand(arraylist_t *A, int files, int std){
    int returnVal = 0; 

    char *program; 
    int done = al_popprogram(A, &program); 
    char exitStr[] = "exit"; 
    char pwdString[] = "pwd";
    char cdString[] = "cd";
    char whichString[] = "which"; 

    char *inFile; 
    char *outFile; 
    int openOut = 1; 
    int openIn = 0;
    int this = al_inFile(A, &inFile); 
    int that = al_outFile(A, &outFile); 

    if (conditional != -1){
        //printf("lastComExit: %d\n", lastComExit); 
        //printf("conditional: %d\n", conditional); 
        if (conditional == 0){
            if (lastComExit != 0){
                conditional = -1; 
                return -1; 
            }
            else {
                //do nothing
            }
        }
        else if (conditional == 1){
            if (lastComExit == 0){
                conditional = -1; 
                return -1;
            }
        }
        else{
            //do nothing
        }
    }
    // printf("made it out of that\n"); 
    // printf("program: %s\n", program); 


    if (outFile != NULL){
        openOut = open(outFile, O_WRONLY|O_TRUNC);
        if (openOut == -1) {
            openOut = open(outFile, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP); 
        }
    }
    if (inFile != NULL){
        openIn = open(inFile, O_RDONLY, O_TRUNC);
    }


    if (openOut == -1){
        printf("could not open file\n"); 
        return -1; 
    }

    //redirect input files if needed
    if (openIn == -1){
        printf("could not open file\n"); 
        return -1; 
    }

    if (std != -1){
        if ((std == 0) && (openIn == 0)){
            openIn = files; 
        }
        if ((std == 1) && (openOut ==1)){
            openOut = files; 
        }
    }

    if (openIn != 0){
        changeArgs(A, openIn);  
        char **args = al_arg(A);
        int length = al_length(A); 
    }

    // char **something = al_arg(A); 
    // for (int i = 0; i < al_length(A); i++){
    //     printf("arg: %s\n", something[i]);
    // }
    // printf("this is openIn: %d\n", openIn); 
    // printf("this is openOut: %d\n", openOut); 


    if (program[0] == '/'){
        returnVal = executePath(A, program, openIn, openOut); 
    }

    else if (program[0] == '.'){
        if (program[1] =='/'){
            returnVal = executePath(A, program, openIn, openOut); 
        }
    }

    else if (strcmp(program, exitStr) == 0){
        returnVal = executeExit(A, openIn, openOut); 
    }
 
    else if (strcmp(program, pwdString) == 0){
        returnVal = executePwd(A, openIn, openOut); 
    }
 
    else if (strcmp(program, cdString) == 0){
        returnVal = executeCd(A, openIn, openOut); 
    }
    
    else if (strcmp(program, whichString) == 0){
        returnVal = executeWhich(A, program, openIn, openOut); 
    }
    
    else {
        returnVal = executeBareName(A, program, openIn, openOut); 
    }

    if (openIn != 0){
        close(openIn); 
    }
    if (openOut != 1){
        close(openOut);
    }

    conditional = -1; 
    return returnVal; 
}

int exec(char *command){
    al_init(&A,1);
    al_init(&B, 1);
    stepThroughCom(&A, command);
    int files[2]; 
    int lastVal; 
    if (pipeGiven == 1){
        int p = pipe(files); 
        if (p < 0){
            lastVal = -1; 
            close(files[1]);
            close(files[0]);
            pipeGiven = 0;  
            al_destroy(&A);
            al_destroy(&B); 
            return lastVal; 
        }
        else {
            //printf("here\n"); 
            lastVal = executeCommand(&A, files[1], 1); 
            close(files[1]); 
            //printf("now here\n"); 
            lastVal = executeCommand (&B, files[0], 0); 
            //printf("made it here ! \n"); 
            close(files[0]); 
            //close(files[1]);
            pipeGiven = 0; 
            al_destroy(&A);
            al_destroy(&B); 
            return lastVal;
        }
    }
    else {
        lastVal = executeCommand(&A, -1, -1); 
    }
  

    al_destroy(&A);
    al_destroy(&B);
    return lastVal; 
}


void readCommand (int fileDescriptor){

    char *buf = (char *) malloc (sizeof(char) *BUFSIZE); 
    for (int i = 0; i < BUFSIZE; i++){
        buf[i] = '\0';
    }
    char *partialCommand;  
    int beginWord; 
    int wordSize; 
    char *beginPtr; 
    int needAppend = 0; 
    int partialComWordSize; 
    char *command; 
    int count = 1; 
    int partialInitialized = 0; 

    //if in interactive mode, give prompt
    if (interactiveMode == 1){
        printf("%s> ", shellName);
        fflush(stdout);
    }

    //fill buffer
    int r = read(fileDescriptor, buf, BUFSIZE);

    while (r > 0 && exitStat != -1){

        beginWord = 0; 
        wordSize = 0; 
        beginPtr = buf;  

        //go through buffer
        for (int i = 0; i < r; i++){

            //if we've reached the end of buffer and the last char is not a new line, create ptr for partial command
            if ((i == r-1) && (buf[i] != '\n')){

                if (needAppend == 1){
                    fflush(stdout);
                    wordSize = i - beginWord + 1; 
                    command = (char *) malloc ((sizeof(char) * (wordSize + partialComWordSize + 1)));
                    memcpy(command, partialCommand, partialComWordSize); 
                    memcpy(command + partialComWordSize, beginPtr, wordSize); 
                    command[wordSize + partialComWordSize] = '\0'; 
                    needAppend = 0; 
                    for (int i = 0; i < strlen(partialCommand); i++){
                        partialCommand[i] = '\0';
                    }
                    free(partialCommand);
                    partialCommand = (char *) malloc ((sizeof(char) * (wordSize + partialComWordSize + 1))); 
                    partialInitialized = 1; 
                    memcpy(partialCommand, command, strlen(command));
                    partialCommand[strlen(command) + 1] = '\0'; 
                    free(command);
                }
                else{
                    wordSize = i - beginWord + 1; 
                    partialCommand = (char *) malloc(sizeof(char) *wordSize + 1); 
                    partialInitialized = 1; 
                    memcpy (partialCommand, beginPtr, wordSize); 
                    partialCommand[wordSize] = '\0';
                    partialComWordSize = wordSize; 
                    needAppend = 1;
                }
            }

            //if we've reached the newline, meaning end of command
            if (buf[i] == '\n'){
                
                //if we have a partially completed command from last buffer fill, append
                if (needAppend ==1){
                    wordSize = i - beginWord; 
                    command = (char *) malloc(sizeof(char) * (wordSize + partialComWordSize + 1));
                    memcpy(command, partialCommand, partialComWordSize); 
                    memcpy (command + partialComWordSize, beginPtr, wordSize); 
                    command[wordSize + partialComWordSize] = '\0'; 
                    beginPtr = beginPtr + wordSize + 1; 
                    beginWord = i + 1; 
                    needAppend = 0; 
                    free(partialCommand);
                }

                else {
                    fflush(stdout);
                    wordSize = i - beginWord; 
                    command = (char *) malloc (sizeof(char) * wordSize +1); 
                    memcpy(command, beginPtr, wordSize); 
                    command[wordSize] = '\0'; 
                    beginPtr = beginPtr + wordSize +1; 
                    beginWord = i + 1; 
                }
                 
                if (interactiveMode == 1){
                    fflush(stdout);
                }
                //exitStat = exitStatus(command);
                lastComExit = exec(command);
                if (exitStat == 1){
                    break;
                }
                //lastComExit = executeCommand(command);
                free(command); 
            }
            
        }

        //exit program if needed
        if (exitStat == 1){
            if (interactiveMode == 1){
                printf("goodbye!\n");
                fflush(stdout);
            }
            break;
        }

        //if in interactive mode, give prompt
        if (interactiveMode ==1){
            printf("%s> ", shellName);
            fflush(stdout);
        }

        //clear buffer
        for (int i  = 0; i < BUFSIZE; i++){
            buf[i] = '\0';
        }
        
        //fill buffer again
        r = read(fileDescriptor, buf, BUFSIZE);
 
    }


    //check to see if there's a partial command waiting to be finished 
    if (partialInitialized == 1){
        if (partialCommand[0] != '\0'){
            if (interactiveMode == 1){
                fflush(stdout); 
            }
            lastComExit = exec(partialCommand);
            for (int i = 0; i < strlen(partialCommand); i++){
                partialCommand[i] = '\0';
            } 
            // free(partialCommand);
            //exitStat = exitStatus(partialCommand); 
            if (exitStat == 1){
                if (interactiveMode == 1){
                    printf("goodbye!\n"); 
                    fflush(stdout); 
                }
                return; 

            }
            else {
                //printf("%d: ", count); 
                //count++; 
                lastComExit = exec(partialCommand);
                // for (int i = 0; i < strlen(partialCommand); i++){
                //     partialCommand[i] = '\0';
                // } 
                free(partialCommand);
            }
        }
    }

    free(buf); 


}
 

int mysh (int argv, char *argc[]){


    if (interactiveMode == 1){
        printf("welcome to my shell!\n"); 
        readCommand(STDIN_FILENO);
        
    }
    else if (argv == 2){
        int file = open(argc[1], O_RDONLY); 
        readCommand(file);
    }
    else {
        //int file = open(STDIN_FILENO, O_RDONLY); 
        readCommand(STDIN_FILENO); 
    }

    return 1; 

}

int main (int argv, char *argc[]){


    if (argv == 2){
        interactiveMode = 0; 
    }
    else  if (isatty(STDIN_FILENO)){
        interactiveMode = 1; 
    }
    else{
        interactiveMode = 0; 
    }
 

    shellName = "mysh"; 
    int real; 

    real = mysh(argv, argc);
   ;
    if (real == 1){
        return EXIT_SUCCESS;
    }

}