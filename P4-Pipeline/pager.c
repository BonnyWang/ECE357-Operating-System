#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAXLINE 23

int main(int argc, char** argv){
    
    ssize_t read;
    size_t len;
    char* line;
    int lineNumber = 0;
    FILE* tty;
    char c;

    tty = NULL;
    
    while((read = getline(&line, &len, stdin)) > 0){
        lineNumber++;
        printf("%s", line);
        if(lineNumber == MAXLINE){
            
            if((tty = fopen("/dev/tty", "r+")) == NULL){
                fprintf(stderr, "Error: Failed to open /dev/tty because %s\n", strerror(errno));
                return -1;
            }

            fprintf(tty, "---Press RETURN for more---");


            while((c = getc(tty)) != EOF){
                if( c == 'q' || c == 'Q'){
                    fprintf(stderr, "*** Pager terminated by Q command ***\n");
                    return 0;
                }

                if(c == '\n'){
                    break;
                }
            }

            if(c < 0){
                fprintf(stderr, "ERROR: fail to get the character from /dev/tty because %s\n", strerror(errno));
            }
            lineNumber =0;
        }
    }

    if(tty != NULL){
        if((fclose(tty)) < 0 ){
            fprintf(stderr, "ERROR: cannot close /dev/tty because %s\n", strerror(errno));
        }

    }

    return 0;
}
