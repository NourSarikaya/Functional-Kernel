#define ASM     1
#include "syscall.h"

#
# void syscall_handler();
#
#       Interface: Arguments are stored in eax, ebx, ecx, and edx.  
#
#          Inputs: eax - Command number 
#                  ebx - first argument  
#                  ecx - second argument 
#                  edx - third argument     
#           
#        Function: System call handler which saves caller saved registers, pushes the arguments in ebx, ecx and edx to the stack, 
#                  analyzes the command number from eax, and jumps to the C function which corresponds to the system call denoted in eax. 
#                  After making the jump, all caller saved registers and restored and IRET is called. 
#
#          Output: A part of the system call path which acts as a dispatcher (allows us to jump to the correct handler for a specific system call).
#
#       Registers: %eax - Command number 
#                  %ebx - first argument  
#                  %ecx - second argument 
#                  %edx - third argument 
# 

.GLOBL syscall_handler                                       
syscall_handler: 

    # check if eax valid [1,10]
    cmpl    $1, %eax                        
    jl      FAIL                           
    cmpl    $10, %eax                      
    jg      FAIL   

    # push caller saved regs 
    PUSHFL

    PUSHL %ebp
    PUSHL %esp
    PUSHL %edi
    PUSHL %esi
 
    # push parameters                                             
    PUSHL %edx
    PUSHL %ecx
    PUSHL %ebx                                                

    # call our specific system call handler                                              
    CALL *sys_call_table(,%eax,4)                          

    # pop caller saved regs
    POPL %ebx
    POPL %ecx
    POPL %edx

    POPL %esi
    POPL %edi
    POPL %esp
    POPL %ebp

    # pop flag
    POPFL                                                    
    
    # jump to LV
    jmp LV

FAIL:
    # store return value of -1
    movl $-1, %eax

LV: 
    # return                                                  
    IRET


# jumptable to the corresponding system call
sys_call_table:
    .long dummy
    .long system_halt
    .long system_execute
    .long system_read
    .long system_write
    .long system_open
    .long system_close
    .long system_getargs
    .long system_vidmap
    .long system_set_handler
    .long system_sigreturn

