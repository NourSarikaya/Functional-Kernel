/*
*   DESCRIPTION OF expnum.h:
*   
*   Every exception/error/system call is accosiated with a certain vector number, which aids in locating the interrupt's gate in the IDT. 
*   These vectors are given below, and are labeled according to what interrupt they represent. 
*
*/

#if !defined(EXPNUM_H)
#define EXPNUM_H

/* IDT VECTORS FOR ALL INTERRUPTS */
/* EXCEPTIONS */
#define EXP_DIV_BY_ZERO    0
#define EXP_DEBUG 1
#define EXP_NONMASK_INTERR 2
#define EXP_BREAKPOINT 3
#define EXP_OVERFLOW 4
#define EXP_BOUND_RANGE_EXCEEDED 5
#define EXP_INAVLID_OP 6
#define EXP_DEVICE_UNAVAIL 7
#define EXP_DOUBLE_FAULT 8
#define EXP_COPROCESSOR_SEGMENT_OVERRUN 9
#define EXP_INVALID_TSS 10
#define EXP_SEGMENT_NOT_PRESENT 11
#define EXP_STACK_SEGEMENT_FAULT 12
#define EXP_GENERAL_PROTECTION_FAULT 13
#define EXP_PAGE_FAULT 14
#define EXP_RESERVED 15
#define EXP_x87_FLOATING_POINT 16
#define EXP_ALIGNMENT_CHECK 17
#define EXP_MACHINE_CHECK   18
#define EXP_SIMD_FLOATING_POINT_EXCEPTION 19

/*PIT*/
#define PIT    0x20
/* KEYBOARD */
#define KEYBOARD_VEC    33

/* RTC */
#define RTC_INTERRUPT 0x28 

/* SYSTEM CALL */
#define SYSTEM_CALL 0x80

#endif /* EXPNUM_H */

