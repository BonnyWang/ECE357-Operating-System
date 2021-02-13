#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h> 
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>

#define MAX 4096
#define WORDMAX 50

int childProcess(char command[MAX][WORDMAX],int commandIndex);

int main(int argc, char* argv[]){
    
    char buf[MAX];
    // int inDescriptor;
    char *line;
    size_t len = 0;
    int byteRead;
    FILE *stream;
    int exit_Status;
    
    char *token;
    char command[MAX][WORDMAX];
    int commandIndex= 0;
    char cwd[MAX];
    int cpid;
    struct rusage rusage;
    clock_t start, end;
    double time_used;

    if(argc == 2){
        if((stream = fopen(argv[1],"r")) < 0){
            fprintf(stderr, "ERROR: unable to open %s because %s\n", argv[1], strerror(errno));
        }
    }else if(argc == 1){
        //Read from stdin
        stream = stdin;
    }else{
        fprintf(stderr, "ERROR: too many arguments");
    }


    while(1){
        if((byteRead = getline(&line, &len, stream)) < 0){
            fprintf(stderr,"end of file read, exiting shell with exit code %d", exit_Status);
            exit(exit_Status);
            // fprintf(stderr,"Fail to read the line because %s\n", strerror(errno));
            // continue;
        }

        if(line[0] == '#'){
            // if the line is a comment
            continue;
        }

        //parse the argument
        commandIndex = 0;
        token = strtok(line, " ");
        
        while(token!= NULL){
            strcpy(command[commandIndex],token);
            token = strtok(NULL, " ");
            commandIndex++;  
        }

        commandIndex--;

        //To remove the newline character at the end
        command[commandIndex][strlen(command[commandIndex])-1] = '\0';

        if(strcmp(command[0],"cd") == 0){
            if(commandIndex == 0){
                strcpy(command[1], getenv("HOME"));
            }else if(commandIndex > 1){
                fprintf(stderr, "ERROR: too many arguments for cd\n");
            }   

            if(chdir(command[1]) < 0){
                fprintf(stderr, "ERROR: fail to change directory because %s\n", strerror(errno));
                continue;
            }
        }else if(strcmp(command[0],"pwd") == 0){
            if(getcwd(cwd, MAX) == NULL){
                fprintf(stderr, "ERROR: faile to get the path because %s\n", strerror(errno));
                continue;
            }
            printf("%s\n",cwd);
            
        
        }else if(strcmp(command[0],"exit") == 0){
            if(commandIndex == 1){
                exit_Status = atoi(command[1]);
            }else if(commandIndex > 1){
                fprintf(stderr,"ERROR: too many arguments for exit!\n");
            }
            exit(exit_Status);
        }else{
            start = clock();
            if(fork() == 0){ 
                childProcess(command, commandIndex);
            }
            cpid = wait3(&exit_Status, 0 , &rusage);
            end = clock();

            time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            if(exit_Status != 0){
				if(WIFSIGNALED(exit_Status)){
					fprintf(stderr,"Child process %d exited with signal %d\n",cpid,WTERMSIG(exit_Status));
				}else{
					fprintf(stderr,"Child process %d exited with return value %d\n",cpid,WEXITSTATUS(exit_Status));
				}
				exit_Status = WEXITSTATUS(exit_Status);
			}else{
				printf("Child process %d exited normally\n", cpid);
			}
            printf("Real: %.3fs User: %d.%03ds Sys: %d.%03ds\n",time_used,rusage.ru_utime.tv_sec,rusage.ru_utime.tv_usec/1000,rusage.ru_stime.tv_sec,rusage.ru_stime.tv_usec/1000);
        }
    }
}

int childProcess(char command[MAX][WORDMAX], int commandIndex){
    char *cmd;
    char *argv[MAX];
    int i;
    char *fileName;
    int fd;

    cmd = command[0];

    for(i = 0; i<= commandIndex; i++){
        argv[i] = command[i];
    }

    //handle IO redirection
    for(i = 0; i <= commandIndex; i++){
        
        if(command[i][0] == '<'){
            // redirect stdin to specified file
            fileName = command[i] + 1;
            if((fd = open(fileName,O_RDONLY)) < 0){
                fprintf(stderr,"ERROR: fail to open %s because %s\n", fileName, strerror(errno));
                exit(1);
            }
            if(dup2(fd, 0) < 0){
                fprintf(stderr,"ERROR: fail to redirect %s to stdin because %s\n", fileName, strerror(errno));
                exit(1);
            }
            
        }else if(command[i][0] == '>'){
            // redirect stdout
            if(command[i][1] == '>'){
                // case of >>filename
                fileName = command[i] + 2;
                if((fd = open(fileName,O_RDWR|O_CREAT|O_APPEND, 0666)) < 0){
                    fprintf(stderr,"ERROR: fail to open %s because %s\n", fileName, strerror(errno));
                    exit(1);
                }
            }else{
                // case of >filename
                fileName = command[i] + 1;
                if((fd = open(fileName,O_RDWR|O_CREAT|O_TRUNC, 0666)) < 0){
                    fprintf(stderr,"ERROR: fail to open %s because %s\n", fileName, strerror(errno));
                    exit(1);
                }
            }

            if(dup2(fd, 1) < 0){
                fprintf(stderr,"ERROR: fail to redirect %s to stdout because %s\n", fileName, strerror(errno));
                exit(1);
            }

        } else if(command[i][0] == '2'){
            if(command[i][1] == '>'){
                if(command[i][2] == '>'){
                    //case of 2>>filename
                    fileName = command[i] + 3;
                    if((fd = open(fileName,O_RDWR|O_CREAT|O_APPEND, 0666)) < 0){
                        fprintf(stderr,"ERROR: fail to open %s because %s\n", fileName, strerror(errno));
                        exit(1);
                    }
                }else{
                    //case of 2>filename
                    fileName = command[i] + 2;
                    if((fd = open(fileName,O_RDWR|O_CREAT|O_TRUNC, 0666)) < 0){
                        fprintf(stderr,"ERROR: fail to open %s because %s\n", fileName, strerror(errno));
                        exit(1);
                    }
                }

                if(dup2(fd, 2) < 0){
                    fprintf(stderr,"ERROR: fail to redirect %s to stderr because %s\n", fileName, strerror(errno));
                    exit(1);
                }   
            }
        }else{
            continue;
        }
        
        if(close(fd) < 0){
                fprintf(stderr,"ERROR: fail to close %s because %s\n", fileName, strerror(errno));
                exit(1);
        }

        // remove the io redirect from the command
        argv[i] = NULL;
    }

    execvp(cmd, argv);
    //the child process ended after execvp exit if success
    fprintf(stderr,"ERROR: fail to exec %s because %s\n", cmd, strerror(errno));
    exit(127);
}