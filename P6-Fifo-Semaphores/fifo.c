#include "fifo.h"

void fifo_init(struct fifo *f) {
	sem_init(&f->sr, 0);
	sem_init(&f->sw, MYFIFO_BUFSIZ);
	sem_init(&f->swk, 1);
	sem_init(&f->srk, 1);
	void *map = mmap(NULL, sizeof(int) * 2 + MYFIFO_BUFSIZ * sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (map == MAP_FAILED) {
		fprintf(stderr, "ERROR: fail to create shared map: %s\n", strerror(errno));
		exit(-1);
	}
	f->start = map;
	*f->start = 0;
	map += sizeof(int);
	f->end = map;
	*f->end = 0;
	map += sizeof(int);
	f->queue = map;
}

void fifo_wr(struct fifo *f, unsigned long d) {
	sem_wait(&f->swk);
	sem_wait(&f->sw);
	f->queue[*f->end % MYFIFO_BUFSIZ] = d;
	(*f->end)++;
	sem_inc(&f->swk);
	sem_inc(&f->sr);
}

unsigned long fifo_rd(struct fifo *f) {
	sem_wait(&f->srk);
	sem_wait(&f->sr);
	unsigned long result = f->queue[*(f->start) % MYFIFO_BUFSIZ];
	(*f->start)++;
	sem_inc(&f->srk);
	sem_inc(&f->sw);
	return result;
}

void fifo_destroy(struct fifo *f) {
	sem_destroy(&f->sr);
	sem_destroy(&f->sw);
	sem_destroy(&f->swk);
	sem_destroy(&f->srk);
	munmap(f->start, sizeof(int) * 2 + MYFIFO_BUFSIZ * sizeof(long));
}
