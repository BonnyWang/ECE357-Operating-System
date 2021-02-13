#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int pipe1[2];
int pipe2[2];
int waiting(int cpid, int exit_Status, struct rusage ru);
int initPipe();

int main(int argc, char *argv[]){

  int cpid[3];
  int pid[3];
  struct rusage ru[3];
  int exitStatus[3];
  int i;

    if(pipe(pipe1) < 0){
        fprintf(stderr, "Failed to create pipe1 because %s\n", strerror(errno));
        return -1;
    }
    if(pipe(pipe2) < 0){
        fprintf(stderr, "Failed to create pipe2 becuase %s\n", strerror(errno));
        return -1;
    }

    

    pid[0] = fork();
    if(pid[0] == 0){
    //   process of wordgen
        char *arg1[3];
        if(dup2( pipe1[1], 1) < 0){
            fprintf(stderr, "Error: Failed to direct stdout of wordgen to the input of pipe1 because %s\n", strerror(errno));
            return -1;
        } 
        arg1[0] = "./wordgen";
        arg1[1] = argv[1];
        arg1[2] = NULL;
        initPipe();

        if(execvp(arg1[0], arg1) < 0){
             fprintf(stderr, "ERROR: fail to exec wordgen because %s\n", strerror(errno));
             return -1;
        }
    }else{

        pid[1] = fork();
        if(pid[1] == 0){
            char* arg2[3];
            arg2[0] = "./wordsearch";
            arg2[1] = "words.txt";
            arg2[2] = NULL;

            //   process of wordsearch
            if(dup2(pipe1[0], 0) < 0){
                fprintf(stderr, "Error: Failed to redirect stdin of wordsearch to the output of pipe1 because %s\n", strerror(errno));
                return -1;
            }
            if(dup2(pipe2[1],1) < 0){
                fprintf(stderr, "Error: Failed to redirect stdout of wordsearch to the intput of pipe2 because %s\n", strerror(errno));
                return -1;
            }

            initPipe();
            if(execvp(arg2[0], arg2) < 0){
                fprintf(stderr, "ERROR: fail to exec wordsearch because %s\n", strerror(errno));
                return -1;
            }
        }else{

            pid[2] = fork();
            if(pid[2] == 0){

                char* arg3[2];
                arg3[0] = "./pager";
                arg3[1] = NULL;

                //   process of pager
                if(dup2(pipe2[0],0) < 0){
                    fprintf(stderr, "Error: Failed to redirect stdin of the pager to the output of pipe2 because %s\n", strerror(errno));
                    return -1;
                }
                
                initPipe();
                if(execvp(arg3[0], arg3) < 0){
                    fprintf(stderr, "ERROR: fail to exec pager because %s\n", strerror(errno));
                    return -1;
                }
            }else{

                for(i = 0; i < 3; i++){
                    cpid[i] = waiting(cpid[i],exitStatus[i],ru[i]);
                    if(cpid[i] == pid[0]){
                        if(close(pipe1[1]) < 0){
                            fprintf(stderr, "ERROR: fail to close write side of pipe 1 because %s\n", strerror(errno));
                        }
			            
                    }
                    if(cpid[i] == pid[1]){
                        if(close(pipe1[0]) < 0){
                            fprintf(stderr, "ERROR: fail to close read side of pipe 1 because %s\n", strerror(errno));
                        }
                        if(close(pipe2[1]) < 0){
                            fprintf(stderr, "ERROR: fail to close write side of pipe 2 because %s\n", strerror(errno));
                        }
                    }

                    if(cpid[i] == pid[2]){
                        if(close(pipe2[0]) < 0){
                            fprintf(stderr, "ERROR: fail to close read side of pipe 2 because %s\n", strerror(errno));
                        }
                    }
                }
            }
        }
    }

}

int waiting(int cpid, int exit_Status, struct rusage ru){
    if((cpid = wait3(&exit_Status, 0 , &ru)) < 0){
        fprintf(stderr, "ERROR: fail to wait for the process because %s\n", strerror(errno));
        return -1;
    }else{
        fprintf(stderr, "Child %d exited with %d\n", cpid, exit_Status);  
    }
    return cpid;

}

int initPipe(){
    if(close(pipe1[0]) < 0){
        fprintf(stderr, "ERROR: failed to close the read end of pipe1 because %s\n", strerror(errno));
        return -1;
    }
    if(close(pipe1[1]) < 0){
        fprintf(stderr, "ERROR: failed to close the write end of pipe1 because %s\n", strerror(errno));
        return -1;
    }
    if(close(pipe2[0]) < 0){
        fprintf(stderr, "ERROR: failed to close the read end of pipe2 because %s\n", strerror(errno));
        return -1;
    }
    if(close(pipe2[1]) < 0){
        fprintf(stderr, "ERROR: failed to close the write end of pipe2 because %s\n", strerror(errno));
        return -1;
    }

    return 0;
}
