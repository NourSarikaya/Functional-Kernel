#ifndef CONTEXT_SWITCH_H
#define CONTEXT_SWITCH_H
#include "types.h"
#include "syscall.h"

/*declare context_switch function that handles context switching in system execute*/
extern int32_t context_switch (pcb_t* pcb, const uint32_t eip, const uint32_t cs, const uint32_t eflags,  const uint32_t esp, const uint32_t ss);

/*declare restore_reg function that restores the registers and important values needed to return to system execute from system halt*/
extern int32_t restore_reg (pcb_t* pcb);

#endif
