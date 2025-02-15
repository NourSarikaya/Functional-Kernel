#ifndef SYSCALL_TESTING_H
#define SYSCALL_TESTING_H
#include "types.h"


/*decalre function that's in charge of parsing the system call*/
extern int32_t syscall_parse(const int num, const uint8_t* command); 

#endif
