#include "sem.h"

int my_procnum;
int SigUsr_Trigger = 0;

void sem_init(struct sem *s, int count) {
	void *map = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (map == MAP_FAILED) {
		fprintf(stderr, "ERROE: cannot creat the shared map in sem_init because %s\n", strerror(errno));
		exit(-1);
	}
	s->lock = map;
	map += sizeof(char);
	s->count = map;
	*(s->count) = count;
	map += sizeof(int);
	s->process_sleeping = map;
	map += sizeof(char) * (N_PROC+1);
	s->process_id = map;
}

int sem_try(struct sem *s) {
	spin_lock(s->lock);
	if(*s->count >0){
		(*s->count)--;
		spin_unlock(s->lock);
		return 1;
	}else{
		spin_unlock(s->lock);
		return 0;
	}

}

void sem_wait(struct sem *s) {
	while (1) {
		spin_lock(s->lock);
		if (*s->count > 0) {
			(*s->count)--;
			spin_unlock(s->lock);
			break;
		} else {
			if (s->process_id[my_procnum] == 0){
				s->process_id[my_procnum] = getpid();
			}
			s->process_sleeping[my_procnum] = 1;
			SigUsr_Trigger = 0;
			spin_unlock(s->lock);
			sigset_t mask, oldmask;
			sigemptyset(&mask);
			sigaddset(&mask, SIGUSR1);
			sigprocmask(SIG_BLOCK, &mask, &oldmask);
			if (!SigUsr_Trigger)
				sigsuspend(&oldmask);
			sigprocmask(SIG_UNBLOCK, &mask, NULL);
		}
	}
}

void sem_inc(struct sem *s) {
	spin_lock(s->lock);
	(*s->count)++;
	for (int i = 0; i < N_PROC + 1; i++) {
		if (s->process_sleeping[i]) {
			s->process_sleeping[i] = 0;
			kill(s->process_id[i], SIGUSR1);
		}
	}
	spin_unlock(s->lock);
}

void sem_destroy(struct sem *s) {
	munmap(s->lock, 4096);
}