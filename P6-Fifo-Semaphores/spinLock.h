#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <sched.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "tas.h"

int tas(volatile char *lock);
int spin_lock(char *lock);
void spin_unlock(char *lock);

#endif