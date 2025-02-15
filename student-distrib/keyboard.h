/* keyboard.h - Defines used in interactions with the keyboard */
/* see https://wiki.osdev.org/PS/2_Keyboard for all commands and reponse*/

/*
From https://wiki.osdev.org/%228042%22_PS/2_Controller:

Using interrupts is easy. When IRQ1 occurs you just read from IO Port 0x60 (there is no need to check bit 0 in the Status Register first), 
send the EOI to the interrupt controllers, and return from the interrupt handler. This being said, you know that the data came from the first PS/2 device 
because you received an IRQ1.

When IRQ12 occurs you read from IO Port 0x60 (there is no need to check bit 0 in the Status Register first), send the EOI to the 
interrupt controller/s, and return from the interrupt handler. You know that the data came from the second PS/2 device because you received an IRQ12.

Unfortunately, there is one problem to worry about. If you send a command to the PS/2 controller that involves a response, 
the PS/2 controller may generate IRQ1, IRQ12, or no IRQ (depending on the firmware) when it puts the "response byte" into the buffer. 
In all three cases, you can't tell if the byte came from a PS/2 device or the PS/2 controller. 

In the no IRQ case, you additionally will need to poll for the byte. Fortunately, you should never need to send a command to the PS/2 
controller itself after initialization (and you can disable IRQs and both PS/2 devices where necessary during initialization).
*/

// We have PS/2 controller configured and initialized by the firmware

/*
IO Port	    Access Type	     Purpose
0x60	    Read/Write	    Data Port
0x64	      Read	     Status Register
0x64	     Write	     Command Register
*/

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "i8259.h"
#include "syscall.h"
#include "paging.h"
#include "lib.h"

// NOTE: keyboard is the first ps2 device (assume single ps2 controller)

/* Ports */
#define PS2_DATA_PORT 0x60                          // Data port for PS2 device
#define KEYBOARD_PORT PS2_DATA_PORT                 // Keyboard being a PS2 device, using the 0x60 port

/* corresponding irq for keyboard */
#define KEYBOARD_IRQ_NUM 1

/* Keyboard Commands */
#define KEYBOARD_RESTART_CMD 0xFF                   // command for restarting keyboard

/* Keyboard Responses */
#define KEYBOARD_ACK 0xFA 
#define KEYBOARD_RESNED 0xFE
#define KEYBOARD_SELFTEST_PASSED 0xAA
#define KEYBOARD_SELFTEST_FAILED1 0xFC
#define KEYBOARD_SELFTEST_FAILED2 0xFD

/* Keycodes and Corresponding Keys */
#define LEFT_CTRL_PRESS 0x1D
#define RIGHT_CTRL_PRESS 0xE0
#define LEFT_CTRL_REL 0x9D
#define RIGHT_CTRL_REL 0xE0
#define CAPS_PRESS 0x3A
#define LEFT_SHIFT_PRESS 0x2A
#define RIGHT_SHIFT_PRESS 0x36
#define LEFT_SHIFT_REL 0xAA
#define RIGHT_SHIFT_REL 0xB6
#define L_PRESS 0x26
#define TAB_PRESS 0x0F
#define ENTER_PRESS 0x1C
#define BACKSPACE_PRESS 0x0E

/* Keycode for Switching Terminals */
#define ALT_PRESS 0x38
#define ALT_REL 0xB8
#define F1_PRESS 0x3B
#define F2_PRESS 0x3C
#define F3_PRESS 0x3D

/* Keyboard Constants */
#define KEYBOARD_MAX_TRY 5                          // maximum try during keyboard restart process, currently not using
#define RELEASE_PERIOD   0xB4

/* Initialize keyboard */
void keyboard_init(void);

/* Simple demo that keyboard works*/
void keyboard_demo(void);

/*enable cursor*/
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);

/*update cursor*/
void update_cursor(int x, int y);

/* Keyboard Handler */
void key_handler(); 

/* Getter for next_index */
int get_next_index(); 

/* Setter for next_index */
void set_next_index(int num);

/* Getter for keyboard buffer */
char* get_keys(); 

/* Initializes data for each terminal */
void terminals_init();

/* Sets the Currently Seen Terminal to a New Value */
void set_seen_terminal_num(int num); 

/* Updates the Cursor Position and Saves This New Position in The Specified Terminal Struct */
void set_pos(int terminal_num_t);

/* Terminal Struct */
typedef struct terminal_t {
    /* Terminal Information */
    char keys[128];                     // Keyboard buffer 
    uint32_t next_index;                // Next Index for keyboard buffer 
    uint32_t cursor[2];                 // Saved cursor position 
    struct pcb_t* last_run_pcb;         // Pointer to the last_run_pcb for the terminal 
} terminal_t;

/* Returns Pointer to Terminal Struct for Specified Terminal */
terminal_t* terminal_data(int num);

/* Returns the Number for the Currently Seen Terminal */
int get_seen_terminal_num();

#endif /* _KEYBOARD_H */
