#ifndef _PIT_H
#define _PIT_H

#include "i8259.h"
#include "tests.h"
#include "types.h"
#include "paging.h"
#include "lib.h"
#include "syscall.h"
#include "keyboard.h"
#include "schedule.h"

#define PIT_IRQ_NUM 0
/* Enables/Disables periodic interrupts from the PIT */
void pit_init(); 
/* Handler for PIT Interrupts */
void pit_handler(); 
/* Getter function for the active terminal */
int get_terminal_being_handled(); 
/* Getter function for the PIT counter */
int get_counter(); 

#define CHANNEL0 0x40                 //Channel 0 data port (read/write)
#define CHANNEL1 0x41                 //   Channel 1 data port (read/write)
#define CHANNEL2 0x42                 //   Channel 2 data port (read/write)
#define MODE_COMMAND_REG 0x43         //   Mode/Command register (write only, a read is ignored)
#endif


