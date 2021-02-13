#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAXLEN 50

void generateWord(){
    char word[MAXLEN];
    int length;
    int j;

    
    //This should ensure the length to be between 3-15 
    length = rand() % 13 + 3;

    for(j = 0; j < length; j++){
        word[j] = 'A' + (rand() % 26);
    }

    word[j] = '\0';
        
    printf("%s\n", word);
}

int main(int argc, char** argv){
    
    int limit;
    

    // To ensure it generate different number
    srand(time(NULL));

    if(argc > 2){
        fprintf(stderr,"Error: Too many input arguments");
        return -1;
    }else if(argc == 2){
        // there is a input valid number for the limit
        limit = atoi(argv[1]);
        if(limit != 0){
            int i;
            for(i = 0; i < limit; i++){
                generateWord();
            }
            fprintf(stderr,"Finished generating %d candidate words\n", limit);
            return 0;
        }
    }

    // limit is 0 or no second argument
    while(1){
        generateWord();
    }
}
