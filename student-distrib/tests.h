#ifndef TESTS_H
#define TESTS_H

//if 1 = rtc interrupts are on 
//if 0 = rtc interrupts are off 
#define RTC_ON 1

/* Test Launcher */
void launch_tests();

/* Exception Tests */
int div_by_zero();
int breakpoint();
int bound_range_exceeded();
int overflow();
int invalid_op();
int device_not_avail();
int coprocessor_seg_overrun();
int x87_floating();
int machine_check();
int SIMD_floating(); 

/* Error Code Tests */
int double_fault();
int invalid_tss();
int seg_not_present();
int stack_seg_fault();
int general_protection_fault();
int page_fault();
int align_check();

/* Paging Test */
int page_test();

/* File Test */
void file_test_f(); 

/* Directory Test */
void directory_test();

/* RTC Tests */
void rtc_test();
void rtc_open_test();
void rtc_bad_hz_test(int freq);
void file_test_ex(); 

/* Terminal Test */
void terminal_test(); 

/* System Call Test (Execute, Halt, Open, Close, Read, Write)*/
int system_call_test();

#endif /* TESTS_H */
