#ifndef _RTC_H
#define _RTC_H

#include "i8259.h"
#include "lib.h"
#include "tests.h"
#include "types.h"


/* Update Rate */
#define UPDATE_RATE 32

/* corresponding irq for rtc */
#define RTC_IRQ_NUM 8


#define NMI_DISABLE 0x80
#define STATUS_REG_A 0x0A
#define STATUS_REG_B 0x0B
#define STATUS_REG_C 0x0C

#define RTC_INDEX_PORT 0x70
#define RTC_DATA_PORT 0x71

#define ENABLE_PERIODIC_INTERRUPT 0x40
#define ENABLE_ALARM_INTERRUPT 0x20
#define ENABLE_UPDATE_ENDED_INTERRUPT 0x10



/* RTC DRIVER FUNCTIONS */
/* Open RTC and set the frequency to 2 Hz */
int32_t rtc_open (const uint8_t* filename);
/* Return after an RTC Interrupt is fired */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);
/* Set the RTC frequency to the frequency passed through the buffer */
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);
/* Close the RTC */
int32_t rtc_close(int32_t fd);

/* Enables/Disables periodic interrupts from the RTC depending on the RTC_ON macro from tests.h */
void rtc_init(); 

/* Handler for RTC Interrupts */
void rtc_handler(); 

#endif







