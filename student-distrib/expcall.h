/*
*   DESCRIPTION OF expcall.h:
*   
*   This file contains the headers for each of the exception/error/system call functions, which execute what should be done in the 
*   event that a certain exception/error/system call is raised; in the case of checkpoint 1, this is printing what exception/error/system call was raised to the screen. 
*   Each of these functions are passed into an assembly macro function in expcall.S, where this macro function takes one of the below functions as an input.  
*
*   FURTHER NOTES: 
*   
*   All calls return >= 0 on success or -1 on failure.
*
*   Note that the system call for halt will have to make sure that only
*   the low byte of EBX (the status argument) is returned to the calling
*   task. Negative returns from execute indicate that the desired program
*   could not be found.
*
*/

#if !defined(EXPCALL_H)
#define EXPCALL_H

/* EXCEPTIONS */
extern void exp_divide_by_zero();
extern void exp_debug();
extern void exp_non_mask_interr();
extern void exp_breakpoint();
extern void exp_overflow();
extern void exp_bound_range_exceeded();
extern void exp_invalid_op();
extern void exp_device_unavail();
extern void exp_coprocessor_seg_overrun();
extern void exp_x87_floating();
extern void exp_machine_check();
extern void exp_SIMD_floating();
extern void keyboard_handler();

/* ERROR CODES */
extern void exp_double_fault();
extern void exp_invalid_tss();
extern void exp_seg_not_present();
extern void exp_stack_seg_fault();
extern void exp_general_protection_fault();
extern void exp_page_fault();
extern void exp_align_check();

/*SYSTEM CALL*/
extern void system_call(); 

/* RTC INTERRUPTS */
extern void rtc_interrupt(); 

/* PIT INTERRUPTS */
extern void pit_interrupt(); 

#endif /* EXPCALL_H */

