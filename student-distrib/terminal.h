#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "keyboard.h"

/*opens the terminal*/
int32_t terminal_open (const uint8_t* filename);
/*terminal reads keyboard*/
int32_t terminal_read(int32_t fd, void* terminal_read_buf, int32_t nbytes);
/*writes terminal buffer content to screen*/
int32_t terminal_write(int32_t fd, const void* terminal_write_buf, int32_t nbytes);
/*closes the terminal*/
int32_t terminal_close(int32_t fd); 
/*getter to obtain terminal buffer*/
extern char* get_terminal_buf();
/*getter to obtain the next available index*/
extern int32_t get_next_avail_index();

#endif

