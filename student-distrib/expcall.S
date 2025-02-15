#   
#   DECRIPTION OF expcall.S 
#
#   This file contains two macro functions - IDT_EXCEPTION_MACRO and IDT_EXCEPTION_MACRO_ERROR. Both of these functions serve to encase our original handler function 
#   [void hander(int vector)] located in idt.c. These macro functions are called whenever the handler needs to be executed when an exception/error/system call is recieved. 
#
#   Note that some exceptions/errors/systems calls push an error code to the top of the stack, which allows for better insight into what type of interrupt has taken place. 
#   So, before returning to the caller function these error codes need to be popped off the stack. As such, any exception/error/system call that DOES NOT push an error code 
#   will call IDT_EXCEPTION_MACRO, whiile any exception/error/system call that DOES push an error code will call IDT_EXCEPTION_MACRO_ERROR. 
#
#   While the handler simply "handles" the interrupts in the IDT, these macro functions first push all general purpose registers (PUSHAL) and the EFLAGS register (PUSHFL) to the 
#   stack. Then, they push the vector value of the current exception/error/systm call recieved and call the handler. Finally, this vector value is popped off the stack along with
#   the general purpose and EFLAGS registers. 
#




#include "expnum.h"

#
#   IDT_EXCEPTION_MACRO
#
#   DESCRIPTION/FUNCTIONALITY: 
#   This function is a wrapper for the interrupts that do NOT push an error code onto the stack and call the handler 
#   given in idt.c. It pushes all general purpose registers, the flags register, and the vector value of the interrupt 
#   onto the stack. It then calls the interrupt handler, and proceeds to pop all information previously pushed onto the stack off 
#   the stack, and then returns to the caller function. 
#
#   INPUTS:   
#   function_name - desired name of the function which will be executed when a specific exception/error/system call is raised
#   handler       - handler function located in idt.c which is the interrupt handler for all exception/error/system calls; the function to be wrapped. 
#   vector        - vector value of the interrupt located within the IDT
#
#   OUTPUTS: 
#   N/a
#
#define IDT_EXCEPTION_MACRO(function_name, handler, vector)  \
.GLOBL function_name                                        ;\
function_name:                                              ;\
    PUSHAL                                                  ;\
    PUSHFL                                                  ;\
    PUSHL $vector                                           ;\
    CALL handler                                            ;\
    ADDL $4, %esp                                           ;\
    POPFL                                                   ;\
    POPAL                                                   ;\
    IRET


# Initialize all wrapper functions for the handler in idt.c - done for each interrupt in the IDT 
IDT_EXCEPTION_MACRO(exp_divide_by_zero, handler, EXP_DIV_BY_ZERO)
IDT_EXCEPTION_MACRO(exp_debug, handler, EXP_DEBUG)
IDT_EXCEPTION_MACRO(exp_non_mask_interr, handler, EXP_NONMASK_INTERR)
IDT_EXCEPTION_MACRO(exp_breakpoint, handler, EXP_BREAKPOINT)
IDT_EXCEPTION_MACRO(exp_overflow, handler, EXP_OVERFLOW)
IDT_EXCEPTION_MACRO(exp_bound_range_exceeded, handler, EXP_BOUND_RANGE_EXCEEDED)
IDT_EXCEPTION_MACRO(exp_invalid_op, handler, EXP_INAVLID_OP)
IDT_EXCEPTION_MACRO(exp_device_unavail, handler, EXP_DEVICE_UNAVAIL)
IDT_EXCEPTION_MACRO(exp_coprocessor_seg_overrun, handler, EXP_COPROCESSOR_SEGMENT_OVERRUN)
IDT_EXCEPTION_MACRO(exp_x87_floating, handler, EXP_x87_FLOATING_POINT)
IDT_EXCEPTION_MACRO(exp_machine_check, handler, EXP_MACHINE_CHECK)
IDT_EXCEPTION_MACRO(exp_SIMD_floating, handler, EXP_SIMD_FLOATING_POINT_EXCEPTION)
IDT_EXCEPTION_MACRO(pit_interrupt, handler, PIT)
IDT_EXCEPTION_MACRO(rtc_interrupt, handler, RTC_INTERRUPT)
IDT_EXCEPTION_MACRO(keyboard_handler, handler, KEYBOARD_VEC)




#
#   IDT_EXCEPTION_MACRO_ERROR
#
#   DESCRIPTION/FUNCTIONALITY: 
#   This function is a wrapper for the interrupts that DO push an error code onto the stack and call the handler 
#   given in idt.c. It pushes all general purpose registers, the flags register, and the vector value of the interrupt 
#   onto the stack. It then calls the interrupt handler, and proceeds to pop all information previously pushed onto the stack off 
#   the stack. Note that these exception/error/system calls produce an error code which is pushed onto the top of the stack. 
#   As such, this error code is popped off along with the rest of the pushed information. After this, the function returns to the caller. 
#
#   INPUTS:   
#   function_name - desired name of the function which will be executed when a specific exception/error/system call is raised
#   handler       - handler function located in idt.c which is the interrupt handler for all exception/error/system calls; the function to be wrapped. 
#   vector        - vector value of the interrupt located within the IDT
#
#   OUTPUTS: 
#   N/a
#
#define IDT_EXCEPTION_MACRO_ERROR(function_name, handler, vector)    \
.GLOBL function_name                                                ;\
function_name:                                                      ;\
    PUSHAL                                                          ;\
    PUSHFL                                                          ;\
    PUSHL $vector                                                   ;\
    CALL handler                                                    ;\
    ADDL $8, %esp                                                   ;\
    POPFL                                                           ;\
    POPAL                                                           ;\
    IRET


# Initialize all wrapper functions for the handler in idt.c - done for each interrupt in the IDT 
IDT_EXCEPTION_MACRO_ERROR(exp_double_fault, handler, EXP_DOUBLE_FAULT)
IDT_EXCEPTION_MACRO_ERROR(exp_invalid_tss, handler, EXP_INVALID_TSS)
IDT_EXCEPTION_MACRO_ERROR(exp_seg_not_present, handler, EXP_SEGMENT_NOT_PRESENT)
IDT_EXCEPTION_MACRO_ERROR(exp_stack_seg_fault, handler, EXP_STACK_SEGEMENT_FAULT)
IDT_EXCEPTION_MACRO_ERROR(exp_general_protection_fault, handler, EXP_GENERAL_PROTECTION_FAULT)
IDT_EXCEPTION_MACRO_ERROR(exp_page_fault, handler, EXP_PAGE_FAULT)
IDT_EXCEPTION_MACRO_ERROR(exp_align_check, handler, EXP_ALIGNMENT_CHECK)

