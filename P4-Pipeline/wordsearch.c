#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFSIZE 500000

int matchedNumber;

void sigHandler(){
    fprintf(stderr, "pid %d received signal 13\n", getpid());
    fprintf(stderr, "Matched %d words\n", matchedNumber);
    exit(0);
}

void upperCase(char *word){
    int i;
    for( i = 0; i < strlen(word); i++){
        word[i] = toupper(word[i]);
    }
}

int main(int argc, char** argv){
    FILE* inFile;
    char* buf[BUFSIZE];
    char *word;
    int index = 0;
    ssize_t nread;
    size_t len = 0;

    signal(SIGPIPE, sigHandler);

    if(argc == 2){
        if((inFile = fopen(argv[1], "r")) == NULL){
            fprintf(stderr, "ERROR: cannot open %s because %s\n", argv[1], strerror(errno));
            return -1;
        }
    }else{
        if(argc < 2){
            fprintf(stderr, "ERROR: Please enter the input dictionary!\n");
        }else{
            fprintf(stderr, "ERROR: Too many arguments!\n");
        }

        return -1;
    }

    // Read the dictionary to buffer 
    while((nread = getline(&buf[index], &len, inFile)) != -1){
        upperCase(buf[index]);
        index++;
    }

    while((nread = getline(&word, &len, stdin)) > 0){
        int i;
        upperCase(word);
        for(i = 0; i < index; i++){
            if(!strcmp(word, buf[i])){
                printf("%s", word);
                matchedNumber++;
                break;
            }
        }
    }
    
    fprintf(stderr, "Accepeted %d words\n", index);
    fprintf(stderr, "Matched %d words\n", matchedNumber);

    if(fclose(inFile) < 0 ){
        fprintf(stderr, "ERROR: cannot close input dictionary because %s\n", strerror(errno));
    }
}