#ifndef SYSCALL_H
#define SYSCALL_H

#ifndef ASM

#include "types.h"
#include "filesystem.h"
#include "keyboard.h"

#define PCB_ARR_MAX_COUNT 6

/*declare all system call functions*/
extern int32_t dummy ();
extern int32_t system_halt (uint8_t status);
extern int32_t system_execute (const uint8_t* command);
extern int32_t system_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t system_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t system_open (const uint8_t* filename);
extern int32_t system_close (int32_t fd);
extern int32_t system_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t system_vidmap (uint8_t** screen_start);
extern int32_t system_set_handler (int32_t signum, void* handler_address);
extern int32_t system_sigreturn (void);

/*sets up pcb struct*/
typedef struct pcb_t {
    /* General Purpose Regs */
    /* as each register is 32 bit, each of them occupy 4 bytes */
    /* thus, offset increment by 4 for every register/ eflags */
    uint32_t ebp; //offset 0x0
    uint32_t eax; //offset 0x4
    uint32_t ebx; //offset 0x8
    uint32_t ecx; //offset 0xC
    uint32_t edx; //offset 0x10
    uint32_t esi; //offset 0x14
    uint32_t edi; //offset 0x18

    /* Flags */
    uint32_t eflags; //offset 0x1C

    /* Ptr to 1st instruction in memory */
    uint32_t eip; //offset 0x20

    /* Stack Ptr */
    uint32_t esp; //offset 0x24

    /* halt status attribute*/
    uint32_t halt_status; //offset 0x28

    /* Other stuff */ 
    uint32_t process_num; 
    uint32_t in_use; 
    uint32_t parent_id; 
    uint32_t inode_num;   
    uint32_t length; 
    file_desc_t fds[8]; 
    int terminal; 
    int initialize_flag;
    int addr; 

} pcb_t;

/*initialize pcb*/
int32_t pcb_init(); 

/*current pcb getter function*/
pcb_t* pcb_ptr();

/*changes global syscall.c pcb to point to the address addr*/
void change_global_pcb(pcb_t* addr); 

#endif /* ASM */

#endif /* SYSCALL_H */
