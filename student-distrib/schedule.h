#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "i8259.h"
#include "tests.h"
#include "types.h"
#include "paging.h"
#include "lib.h"
#include "syscall.h"
#include "keyboard.h"
#include "schedule.h"
#include "x86_desc.h"

/* schedule function itself*/
extern void schedule(int terminal_being_handled);

#endif
