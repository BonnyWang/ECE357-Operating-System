#include "spinLock.h"

int spin_lock(char *lock) {
    while (tas(lock)) {
        if (sched_yield() < 0) {
            fprintf(stderr, "ERROR: yielding cpu uncorrectly because %s\n", strerror(errno));
            return -1;
        }
    }
    return 1;
}

void spin_unlock(char *lock) {
    *lock = 0;
}
