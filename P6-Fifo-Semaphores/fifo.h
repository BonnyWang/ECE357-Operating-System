#ifndef FIFO_H
#define FIFO_H


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "sem.h"

#define MYFIFO_BUFSIZ 4096



struct fifo {
	struct sem sr, sw, srk, swk;
	int *start, *end;
	unsigned long *queue;
};

void fifo_init(struct fifo *f);
void fifo_wr(struct fifo *f,unsigned long d);
unsigned long fifo_rd(struct fifo *f);
void fifo_destroy(struct fifo *f);

#endif