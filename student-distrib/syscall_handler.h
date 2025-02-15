#ifndef SYSCALL_START_H
#define SYSCALL_START_H
#include "types.h"
#ifndef ASM

/*declare the syscall_handler that handle the system calls*/
extern void syscall_handler(); 
#endif
#endif
