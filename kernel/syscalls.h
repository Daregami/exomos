#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

void syscall_handler(uint32_t vector);

#endif