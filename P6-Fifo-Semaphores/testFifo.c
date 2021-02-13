#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "sem.h"
#include "fifo.h"

#define MAX 1000000

void SigUsrHandler(int signum);

extern int my_procnum;
extern int SigUsr_Trigger;


int main(){
	
    if (signal(SIGUSR1, &SigUsrHandler) == SIG_ERR) {
		fprintf(stderr, "ERROE: fail to create signal handler because %s\n", strerror(errno));
		return -1;
	}

	struct fifo f;
	fifo_init(&f);
	printf("When there are a single reader and a single writer\n");
	my_procnum = 0;
	if (fork() == 0) {
		for(int i = 0; i < MAX; i++)
			fifo_wr(&f, i);
		exit(0);
	}
	my_procnum = 1;
	int r_error = 0;
	for (int i = 0; i < MAX; i++) {
		if (fifo_rd(&f) != i) {
			r_error = 1;
			break;
		}
	}
	if (r_error) {
		fprintf(stderr,"ERROR: fail to read!\n");
	} else {
		printf("FIFO read success!\n");
	}
	fifo_destroy(&f);

	printf("When there are a single reader and multiple writers\n");
	fifo_init(&f);
	for (my_procnum = 0; my_procnum < N_PROC; my_procnum++) {
		if (fork() == 0) {
			for (int i = 0; i < MAX; i++) {
				fifo_wr(&f, ((unsigned long)my_procnum<< 32) + i);
			}
			exit(0);
		}
	}
	
	int counter[N_PROC];
	memset(&counter, 0, sizeof(counter));
	
	r_error = 0;
	for (int i = 1; i <= N_PROC*MAX; i++) {
		unsigned long rd = fifo_rd(&f);
		int pid = rd >> 32;
		int number = rd & 0xFFFFFFFF;
		printf("Current : %d\n", i);
		if (counter[pid]++ != number) {
			r_error = 1;
			break;
		}
	}
	if (r_error) {
		fprintf(stderr, "ERROR: fail to read!\n");
	} else {
		printf("FIFO Read success!\n");
	}

    return 0;
}

void SigUsrHandler(int signum) {
	SigUsr_Trigger = 1;
}