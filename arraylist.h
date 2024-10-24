typedef struct {
    char *program;
    char **data;
    int redirectIn; 
    int redirectOut; 
    int args; 
    char *redirectInFile;
    char *redirectOutFile; 
    unsigned length;
    unsigned capacity;
    //struct *next;
} arraylist_t;

void al_init(arraylist_t *, unsigned);
void al_destroy(arraylist_t *);
int al_redirectIn(arraylist_t *);
int al_redirectOut(arraylist_t *L); 
int al_inFile(arraylist_t *, char **);
int al_outFile(arraylist_t *, char **);
void al_addInFile(arraylist_t *, char *);
void al_addOutFile(arraylist_t *, char *);
unsigned al_length(arraylist_t *);
int al_argsNum(arraylist_t *);
void al_pushprogram(arraylist_t *, char *);
int al_popprogram(arraylist_t *, char**);
void al_push(arraylist_t *, char*);
int al_pop(arraylist_t *, char **);
char** al_arg(arraylist_t *); 
