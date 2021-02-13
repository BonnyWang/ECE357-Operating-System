#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "spinLock.h"

int main() {
    printf("Without a splinLock\n");
    void *shared_region;
    if ((shared_region = mmap(NULL, 100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0)) < 0) {

        fprintf(stderr, "ERROR: Fail to mmap to the region %s\n", strerror(errno));
        return-1;
    }

    unsigned long long *shared_value = (unsigned long long *)shared_region;
    *shared_value = 0;

    for(int i = 0; i < 6; i++) {
        if (fork() == 0) {
            for (int j = 0; j < 1000000; j++){
                *shared_value += 1;
            }
            exit(0);
        }
    }

    while (wait(NULL) > 0);

    printf("The shared value is %lld\n", *shared_value);

    printf("With a spinLock\n");
    typedef struct account {
        char lock;
        unsigned long long val;
    } account;
    
    account *maccount = (account *)shared_region;
    maccount->val = 0;
    maccount->lock = 0;

    for (int i = 0 ; i < 6 ; ++i) {
        if (fork() == 0) {
            for (int j=0; j < 1000000; ++j) {
                spin_lock(&maccount->lock);
                maccount->val += 1;
                spin_unlock(&maccount->lock);
            }
            exit(0);
        }
    }

    while (wait(NULL) > 0);
    
    printf("The shared value is %lld\n", maccount->val);
}