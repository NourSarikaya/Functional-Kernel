/* keyboard.c - Initializes communication with keyboard and includes keybaord demo. */

#include "keyboard.h"
#include "lib.h"


/* TABLE MAPPING KEYCODES TO CHARACTERS */
/* NOTE: For keys like escape, F1, etc., we print '' (aka nothing) to the screen */
//  0X00    nothing             0x01	escape              0x02	1 pressed	        0x03	2 pressed
//  0x04	3 pressed	        0x05	4 pressed	        0x06	5 pressed	        0x07	6 pressed
//  0x08	7 pressed	        0x09	8 pressed	        0x0A	9 pressed	        0x0B	0 (zero) pressed
//  0x0C	- pressed	        0x0D	= pressed	        0x0E	backspace	        0x0F	tab pressed
//  0x10	Q pressed	        0x11	W pressed	        0x12	E pressed	        0x13	R pressed
//  0x14	T pressed	        0x15	Y pressed	        0x16	U pressed	        0x17	I pressed
//  0x18	O pressed	        0x19	P pressed	        0x1A	[ pressed	        0x1B	] pressed
//  0x1C	enter	            0x1D	left control	    0x1E	A pressed	        0x1F	S pressed
//  0x20	D pressed	        0x21	F pressed	        0x22	G pressed	        0x23	H pressed
//  0x24	J pressed	        0x25	K pressed	        0x26	L pressed	        0x27	 ; pressed
//  0x28	' (single quote)	0x29	` (back tick)	    0x2A	left shift	        0x2B	\ pressed
//  0x2C	Z pressed	        0x2D	X pressed	        0x2E	C pressed	        0x2F	V pressed
//  0x30	B pressed	        0x31	N pressed	        0x32	M pressed	        0x33	, pressed
//  0x34	. pressed	        0x35	/ pressed	        0x36	right shift	        0x37	(keypad) *
//  0x38	left alt        	0x39	space       	    0x3A	CapsLock 	        0x3B	F1  
//  0x3C	F2              	0x3D	F3          	    0x3E	F4          	    0x3F	F5  
//  0x40	F6              	0x41	F7          	    0x42	F8          	    0x43	F9  
//  0x44	F10             	0x45	NumberLock  	    0x46	ScrollLock  	    0x47	(keypad) 7
//  0x48	(keypad) 8      	0x49	(keypad) 9     	    0x4A	(keypad) -  	    0x4B	(keypad) 4 
//  0x4C	(keypad) 5      	0x4D	(keypad) 6  	    0x4E	(keypad) +  	    0x4F	(keypad) 1  
//  0x50	(keypad) 2      	0x51	(keypad) 3  	    0x52	(keypad) 0  	    0x53	(keypad) . 
//  0x57	F11                 0x58	F12                 Upper_nothing               Upper_escape
//  Upper_1                     Upper_2                     Upper_3                     Upper_4
//  Upper_5                     Upper_6                     Upper_7                     Upper_8
//  Upper_9                     Upper_0                     Upper_-                     Upper_=
//  Upper_backspace             Upper_tab                   Upper_Q                     Upper_W
//  Upper_E                     Upper_R                     Upper_T                     Upper_Y
//  Upper_U                     Upper_I                     Upper_O                     Upper_P
//  Upper_[                     Upper_]                     Upper_enter                 Upper_left control
//  Upper_A                     Upper_S                     Upper_D                     Upper_F
//  Upper_G                     Upper_H                     Upper_J                     Upper_K
//  Upper_L                     Upper_;                     Upper_'                     Upper_`
//  Upper_left shift            Upper_\                     Upper_Z                     Upper_X
//  Upper_C                     Upper_V                     Upper_B                     Upper_N
//  Upper_M                     Upper_,                     Upper_.                     Upper_/
//  Upper_right shift           Upper_(keypad) *            Upper_left alt              Upper_space
//  Upper_capslock              Upper_F1                    Upper_F2                    Upper_F3
//  Upper_F4                    Upper_F5                    Upper_F6                    Upper_F7
//  Upper_F8                    Upper_F9                    Upper_F10                   Upper_numberlock
//  Upper_scroll_lock           Upper_(keypad)7             Upper_(keypad)8             Upper_(keypad)9
//  Upper_(keypad)-             Upper_(keypad)4             Upper_(keypad)5             Upper_(keypad)6
//  Upper_(keypad)+             Upper_(keypad)1             Upper_(keypad)2             Upper_(keypad)3
//  Upper_(keypad)0             Upper_(keypad).             Upper_F11                   Upper_F12

char scankey_to_character[172] = {
    '_','\0','1','2',           
    '3','4','5','6',            
    '7','8','9','0',           
    '-','=','\b','\t',          
    'q','w','e','r',            
    't','y','u','i',            
    'o','p','[',']',                        
    '\n','\0','a','s',          
    'd','f','g','h',            
    'j','k','l',';',            
    '\'','`','\0','\\',         
    'z','x','c','v',            
    'b','n','m',',',            
    '.','/','\0','\0',          
    '\0',' ','\0','\0',         
    '\0','\0','\0','\0',        
    '\0','\0','\0','\0',        
    '\0','\0','\0','\0',          
    '\0','\0','\0','\0',        
    '\0','\0','\0','\0',         
    '\0','\0','\0','\0',        
    '\0','\0','_','\0',         
    '!','@','#','$',            
    '%', '^', '&', '*',         
    '(', ')','_', '+',          
    '\b', '\t','Q','W',        
    'E','R','T', 'Y',           
    'U', 'I', 'O','P',          
    '{', '}', '\n','\0',        
    'A','S','D','F',            
    'G','H','J','K',            
    'L',':','"','~',            
    '\0','|','Z','X',           
    'C','V','B','N',            
    'M','<','>','?',            
    '\0','\0','\0',' ',         
    '\0','\0','\0','\0',        
    '\0','\0','\0','\0',        
    '\0','\0','\0','\0',        
    '\0','\0','\0','\0',        
    '\0','\0','\0','\0',        
    '\0','\0','\0','\0',        
    '\0','\0','\0','\0'         
};


 /* Global Variables Used for the 3 Terminals */ 
    terminal_t all_terminal_data[3];                // Keyboard Buffers for the 3 Terminals
    static int terminal_num = 1; 


/* Global Variables Used in key_handler() */ 
    int alt_pressed = 0; 
    int capslock = 0;
    int shift = 0; 
    int control = 0; 
    char null = '\0';
    char backspace = '\b';
    uint32_t demo;


/* 
 * terminal_data(int num)
 *   DESCRIPTION: Returns a pointer to the terminal struct for terminal #num.  
 *   INPUTS: num - specific terminal struct we want to access 
 *   OUTPUTS: N/A
 *   RETURN VALUE: pointer to a terminal struct for the desired input terminal 
 *   SIDE EFFECTS: none 
 */
terminal_t* terminal_data(int num){
        return all_terminal_data + (num-1);
    }


/* 
 * terminals_init()
 *   DESCRIPTION: Initialize all terminal structs within our terminal array 
 *   INPUTS: none
 *   OUTPUTS: N/A
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none 
 */
void terminals_init(){
    int i; 
    int j; 
    for(i=0; i<3; i++){                                 // Loop through all 3 terminal structs
        for(j=0; j<128; j++){                           
            all_terminal_data[i].keys[j] = '\0';        // Set the keyboard buffer for each struct to all NULL chars 
        }
        all_terminal_data[i].next_index = 0;            // Initialize next_index, cursor position to 0 
        all_terminal_data[i].cursor[0] = 0; 
        all_terminal_data[i].cursor[1] = 0; 
        all_terminal_data[i].last_run_pcb = NULL;       // Set last run pcb to NULL 
    }
    
}


/* 
 * set_seen_terminal_num(int num)
 *   DESCRIPTION: Sets the global variable which stores the currently seen terminal number to a new value. 
 *   INPUTS: num - number to set the global seen terminal number to
 *   OUTPUTS: N/A
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none 
 */
void set_seen_terminal_num(int num){
    terminal_num = num; 
}


/* 
 * get_seen_terminal_num()
 *   DESCRIPTION: Returns the terminal number associated with the currently seen terminal
 *   INPUTS: none
 *   OUTPUTS: N/A
 *   RETURN VALUE: currently seen terminal number 
 *   SIDE EFFECTS: none 
 */
int get_seen_terminal_num(){
    return terminal_num; 
}


/* 
 * keyboard_init()
 *   DESCRIPTION: Initializes communication with the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Links keyboard interrupts to IR1
 */
void keyboard_init(){
    // Send data to the PS/2 Data Port to reset the Controller
    // not needed for this MP as we are running an emurater
    // outb(KEYBOARD_PORT, KEYBOARD_RESTART_CMD);
    // Link keyboard interrupts to IR1
    enable_irq(1);
}


/* 
 * get_next_index()
 *   DESCRIPTION: Getter function to obtain the next_index (next index to store a char in our keyboard buffer)  
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: next_index
 *   SIDE EFFECTS: none
 */
int get_next_index(){
    return all_terminal_data[terminal_num-1].next_index; 
}


/* 
 * set_next_index()
 *   DESCRIPTION: Setter function to change the next_index (next index to store a char in our keyboard buffer)  
 *   INPUTS: num - value to change next_index to 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: none
 */
void set_next_index(int num){
    all_terminal_data[terminal_num-1].next_index=num; 
}


/* 
 * get_keys()
 *   DESCRIPTION: Getter function that returns the keys buffer for the current seen terminal (buffer that stores what is read from the keyboard)  
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to keys (keyboard buffer) for currently seen terminal 
 *   SIDE EFFECTS: none
 */
char* get_keys(){
    return all_terminal_data[terminal_num-1].keys;
}



/* 
 * enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
 *   DESCRIPTION: Enables a cursor to appear in the terminal. 
 *   INPUTS: cursor_start - the highest possible scanline for the cursor 
 *           cursor_end   - the lowest possible scanline for the cursor 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: puts a cursor on the screen 
 *   RESOURCES: https://wiki.osdev.org/Text_Mode_Cursor
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
	outb(0x0A, 0x3D4);                                      // Sends data (0x0A) to port 0x3D4 which handles cursor initialization of highest scanline  
	outb((inb(0x3D5) & 0xC0) | cursor_start, 0x3D5);        // Takes data recieved from port 0x3D5 and isolates the top 2 bits (0xC0) and ORs with highest possible scanline, sends this data back to the port (0x3D5)
 
	outb(0x0B, 0x3D4);                                      // Sends data (0x0B) to port 0x3D4 which handles cursor initialization of lowest scanline  
	outb((inb(0x3D5) & 0xE0) | cursor_end, 0x3D5);          // Takes data recieved from port 0x3D5 and isolates the top 3 bits (0xE0) and ORs with lowest possible scanline, sends this data back to the port (0x3D5)
}


/* 
 * update_cursor(int x, int y)
 *   DESCRIPTION: Puts the cursor at (x,y) on the screen. 
 *   INPUTS: x - desired x position to put the cursor 
 *           y - desired y position to put the cursor 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Puts the cursor at a desired location on the screen. 
 *   RESOURCES: https://wiki.osdev.org/Text_Mode_Cursor
 */
void update_cursor(int x, int y)
{
	uint16_t pos = y * 80 + x;                              // Translates (x,y) coordinates in terminal to 1D coordinates (NOTE: 80 is the width in chars of the screen)
	outb(0x0F, 0x3D4);                                      // Sending data to port 0x3D4 to signal that the cursor position will be updated 
	outb((uint8_t) (pos & 0xFF), 0x3D5);                    // Takes the bottom 8 bits of the 1D position data above and sends this to port 0x3D5
	outb(0x0E, 0x3D4);                                      // Sending data to port 0x3D4 to signal that the cursor position will be updated 
	outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);             // Takes the top 8 bits of the 1D position data above and sends this to port 0x3D5
    all_terminal_data[terminal_num-1].cursor[0]=x;
    all_terminal_data[terminal_num-1].cursor[1]=y;
}


/* 
 * key_handler()
 *   DESCRIPTION: Prints a character to the screen when its correpsponding key is pressed.  
 *   INPUTS: none  
 *   OUTPUTS: Prints data from keyboard to the screen. 
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Prints data from keyboard to the screen. 
 */
void key_handler(){

    /* Handler Preparation */
    mask_and_ack(KEYBOARD_IRQ_NUM);                                 // Disable Interrupts 
    int clear_flag = 0;                                             // Indicates whether screen needs to be cleared

    /* Set cursor */
    update_cursor(get_screen_x(), get_screen_y());                  // Puts the cursor in the correct position 
    
    /* Reading from Keyboard */ 
    demo = inb(KEYBOARD_PORT);                                      // Read keypress from keyboard
    
    /* Check if ENTER is Pressed */               
    if(demo==ENTER_PRESS){
        all_terminal_data[terminal_num-1].keys[all_terminal_data[terminal_num-1].next_index]='\n';                                      // Store \n in the keys buffer 
        if(get_screen_y()==24){                                     // If at the bottom of the screen, vertical shift 
            vertical_shift();
        }
        else{
            printf("%c",'\n');                                      // If not at the bottom of the screen, go to next line 
        }
    }

    /* Check if BACKPSACE is pressed when the BUFFER IS FULL */
    else if(demo==BACKSPACE_PRESS && (all_terminal_data[terminal_num-1].next_index>=128)){              // 128 = size of our buffer 
        printf("%c",scankey_to_character[(int)demo + (capslock*(0x56)) + (shift*(0x56))]);              // Print BACKSPACE to the screen 
        all_terminal_data[terminal_num-1].keys[all_terminal_data[terminal_num-1].next_index-1]='\0';    // Remove the last character (\n) in the buffer as well as the char before that 
        all_terminal_data[terminal_num-1].keys[all_terminal_data[terminal_num-1].next_index-2]='\0';
        all_terminal_data[terminal_num-1].next_index=all_terminal_data[terminal_num-1].next_index-2;    // Decrement next_index by 2
    }

    /* Do not BACKSPACE if we have backspaced the entire buffer */
    else if(demo==BACKSPACE_PRESS && (all_terminal_data[terminal_num-1].next_index==0)){
        enable_irq(KEYBOARD_IRQ_NUM);
        return;
    }

    /* Check if BUFFER NOT FULL and if ANY OTHER KEY is pressed */
    else if(all_terminal_data[terminal_num-1].next_index<128){
        if(demo==LEFT_CTRL_PRESS || demo==RIGHT_CTRL_PRESS){            // Control pressed
            control = 1; 
        }
        else if(demo==LEFT_CTRL_REL || demo==RIGHT_CTRL_REL){           // Control released
            control=0; 
        }
        else if(demo==CAPS_PRESS){                                      // Capslock pressed
            capslock = !capslock;
        }      
        else if(demo==LEFT_SHIFT_PRESS || demo==RIGHT_SHIFT_PRESS){     // Shift pressed 
            shift=1; 
        }
        else if(demo==LEFT_SHIFT_REL || demo==RIGHT_SHIFT_REL){         // Shift released 
            shift=0; 
        }
        else if(demo==ALT_PRESS){                                       // Alt pressed 
            alt_pressed=1;
        }
        else if(demo==ALT_REL){                                         // Alt released 
            alt_pressed=0;
        }
        
        // NOTE: Indices 16-25, 30-38, 44-50 are alphabet keycodes
        // If Capslock and Shift are on and we are not printing a letter  
        if(capslock==1 && shift==1 && (demo <= 0x58) && !((demo >= 16 && demo <= 25) || (demo >= 30 && demo <= 38) || (demo >= 44 && demo <= 50))){
            all_terminal_data[terminal_num-1].keys[all_terminal_data[terminal_num-1].next_index]=scankey_to_character[(int)demo + 0x56];        // Capitalize the character 
            if(demo != BACKSPACE_PRESS){
                all_terminal_data[terminal_num-1].next_index++;
            }
            if(demo == BACKSPACE_PRESS){
                all_terminal_data[terminal_num-1].next_index--;
            }
            printf("%c",scankey_to_character[(int)demo + 0x56]);            // Print the character to kernel. Cast the value inputted from keyboard to int and find the corresponding index in our array
        }

        // NOTE: Indices 16-25, 30-38, 44-50 are alphabet keycodes
        // If Capslock and Shift are on and we are printing a letter 
        if(capslock==1 && shift==1 && ((demo >= 16 && demo <= 25) || (demo >= 30 && demo <= 38) || (demo >= 44 && demo <= 50))){    
            shift=-1;                                                       // Make shift = -1 -> this allows the index to be correct when accessing the character table above 
        }

        /* Check if CTRL - L is Pressed */
        if(control==1 && demo==L_PRESS){                                
            clear();                                                        // Clear the screen and set the clear_flag to 1
            clear_flag=1;
        }

        /* Check if ALT - F1 is Pressed (switch to Terminal 1) */
        if(alt_pressed==1 && demo==F1_PRESS){                                                                                           
            memcpy(terminal_addr_ptr(terminal_num), get_vid_mem_ptr(), 4096);                   // Store what is in video memory into the page of the terminal we are LEAVING 
            all_terminal_data[terminal_num-1].last_run_pcb = pcb_ptr();                         // Store the last_run_pcb into the terminal struct for the terminal we are LEAVING
            terminal_num=1;                             
            set_screen_x(all_terminal_data[terminal_num-1].cursor[0]);                          // Set the position of the cursor such that is aligns with the terminal we are ENTERING
            set_screen_y(all_terminal_data[terminal_num-1].cursor[1]);
            memcpy(get_vid_mem_ptr(), terminal_addr_ptr(terminal_num), 4096);                   // Store what is in the page of the terminal we are ENTERING into video memory 
            //load_4MB_syscall_page(0);                                                         // (SCHEDULING) - Map the Program Image to the Shell for Terminal 1
            //change_global_pcb(all_terminal_data[0].last_run_pcb);                             // (SCHEDULING) - Change the global pcb in syscall.c to be the last_run_pcb of terminal 1 
        }

        /* Check if ALT - F2 is Pressed (switch to Terminal 2) */
        if(alt_pressed==1 && demo==F2_PRESS){                                                                                                        
            memcpy(terminal_addr_ptr(terminal_num), get_vid_mem_ptr(), 4096);                   // Store what is in video memory into the page of the terminal we are LEAVING 
            all_terminal_data[terminal_num-1].last_run_pcb = pcb_ptr();                         // Store the last_run_pcb into the terminal struct for the terminal we are LEAVING
            terminal_num=2; 
            set_screen_x(all_terminal_data[terminal_num-1].cursor[0]);                          // Set the position of the cursor such that is aligns with the terminal we are ENTERING
            set_screen_y(all_terminal_data[terminal_num-1].cursor[1]);
            memcpy(get_vid_mem_ptr(), terminal_addr_ptr(terminal_num), 4096);                   // Store what is in the page of the terminal we are ENTERING into video memory
            //load_4MB_syscall_page(1);                                                         // (SCHEDULING) - Map the Program Image to the Shell for Terminal 2
            //change_global_pcb(all_terminal_data[1].last_run_pcb);                             // (SCHEDULING) - Change the global pcb in syscall.c to be the last_run_pcb of terminal 2 
        }

        /* Check if ALT - F3 is Pressed (switch to Terminal 3) */
        if(alt_pressed==1 && demo==F3_PRESS){                                                                                                         
            memcpy(terminal_addr_ptr(terminal_num), get_vid_mem_ptr(), 4096);                   // Store what is in video memory into the page of the terminal we are LEAVING
            all_terminal_data[terminal_num-1].last_run_pcb = pcb_ptr();                         // Store the last_run_pcb into the terminal struct for the terminal we are LEAVING
            terminal_num=3;
            set_screen_x(all_terminal_data[terminal_num-1].cursor[0]);                          // Set the position of the cursor such that is aligns with the terminal we are ENTERING
            set_screen_y(all_terminal_data[terminal_num-1].cursor[1]);
            memcpy(get_vid_mem_ptr(), terminal_addr_ptr(terminal_num), 4096);                   // Store what is in the page of the terminal we are ENTERING into video memory
            //load_4MB_syscall_page(2);                                                         // (SCHEDULING) - Map the Program Image to the Shell for Terminal 3
            //change_global_pcb(all_terminal_data[2].last_run_pcb);                             // (SCHEDULING) - Change the global pcb in syscall.c to be the last_run_pcb of terminal 3
        }
         
        /* Check of TAB is Pressed and we are at the bottom right corner of the screen */
        if(demo==TAB_PRESS && get_screen_y()==24 && get_screen_x()>=76){
            all_terminal_data[terminal_num-1].keys[all_terminal_data[terminal_num-1].next_index]='\t';                                          // Store tab in our buffer 
            int x = get_screen_x();                                         // Save original screen_x 
            vertical_shift();                                               // Vertical Shift if this is the case 
            set_screen_x((x+4)%80);                                         // Restore to correct screen_x value on our new line 
            all_terminal_data[terminal_num-1].next_index++; 
            enable_irq(KEYBOARD_IRQ_NUM);
            return;
        }
        int flag_period=0;
        if(demo == RELEASE_PERIOD){ // release code for . triggers backspacing
            // flag_. sets to 1, serves as a check that no backspace is being entered
            flag_period=1;
        }


        // We only want to print the character to the screen if screen was NOT cleared, when the key is PRESSED (not released), and when the key SHOULD print something (not null)
        // NOTE: Keycodes<=0x58 are PRESS codes. Keycodes>0x58 are RELEASE codes.
        // NOTE: 0x56 is our offset to access the UPPERCASE version of each key
        if((clear_flag==0) && (demo <= 0x58) && ((int)scankey_to_character[(int)demo + (capslock*(0x56)) + (shift*(0x56))] != (int)null) && ((int)scankey_to_character[(int)demo + (capslock*(0x56)) + (shift*(0x56))] != (int)backspace)){        
            
            // If the keycode is not a letter, our buffer is not full, and shift is not pressed 
            if(!((demo >= 16 && demo <= 25) || (demo >= 30 && demo <= 38) || (demo >= 44 && demo <= 50))&& all_terminal_data[terminal_num-1].next_index<127 && ((shift == 0 && capslock == 0) || (shift == 0 && capslock == 1))){
               all_terminal_data[terminal_num-1].keys[all_terminal_data[terminal_num-1].next_index]=scankey_to_character[(int)demo];
                printf("%c",scankey_to_character[(int)demo]);                             // Print the character to kernel. Cast the value inputted from keyboard to int and find the corresponding index in our array
            }
            
            // If the keycode is not an alphabet AND the buffer is not full  AND Shift/Capslock are equal to 1
            else if(!((demo >= 16 && demo <= 25) || (demo >= 30 && demo <= 38) || (demo >= 44 && demo <= 50)) && all_terminal_data[terminal_num-1].next_index<127 && ((shift == 1 && capslock == 1))){

                if(!(demo == 52 || demo == 53 || demo == 62 || demo == 63)) {                       // Corner cases of last two keycodes (52,53,62,63) ->do not print duplicates
                    /*addes offset of 0x56 to access the second half of the keycode array*/
                    all_terminal_data[terminal_num-1].keys[all_terminal_data[terminal_num-1].next_index]=scankey_to_character[(int)demo + 0x56];
                    printf("%c",scankey_to_character[(int)demo + 0x56]);                            // Print the character to kernel. Cast the value inputted from keyboard to int and find the corresponding index in our array
                }
                else if((demo == 52 || demo == 53 || demo == 62 || demo == 63)) {                   // Corner cases of last two keycodes (52,53,62,63) -> remove space duplicates   
                    /*decrerment next_index*/
                    all_terminal_data[terminal_num-1].next_index--;
                }
            }
            // If the above conditions are not met, print and store the character as normal 
            else if(all_terminal_data[terminal_num-1].next_index<127){
                all_terminal_data[terminal_num-1].keys[all_terminal_data[terminal_num-1].next_index]=scankey_to_character[(int)demo + (capslock*(0x56)) + (shift*(0x56))];
                printf("%c",scankey_to_character[(int)demo + (capslock*(0x56)) + (shift*(0x56))]);   // Print the character to kernel. Cast the value inputted from keyboard to int and find the corresponding index in our array
            }
            // If the buffer is full, but \n in the last available slot 
            else if(all_terminal_data[terminal_num-1].next_index==127){
                all_terminal_data[terminal_num-1].keys[all_terminal_data[terminal_num-1].next_index]='\n';
            }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
            all_terminal_data[terminal_num-1].next_index++;                                                                            // Increment next index 
        }
        
        // If BACKSPACE is pressed when the buffer is NOT full
        if (flag_period==0){
            if(((int)scankey_to_character[(int)demo + (capslock*(0x56)) + (shift*(0x56))] == (int)backspace)){
                printf("%c",scankey_to_character[(int)demo + (capslock*(0x56)) + (shift*(0x56))]);                                  // Print backspace to the screen 
                all_terminal_data[terminal_num-1].next_index=all_terminal_data[terminal_num-1].next_index-1;                        // Decrement next_index 
                all_terminal_data[terminal_num-1].keys[all_terminal_data[terminal_num-1].next_index]='\0';                          // "erase" what was originally in next_index 
            }
        }

        // Restore shift back to 1 if it was changed to -1 
        if(shift==-1){                                                  
            shift=1;
        }

        // If we are at the bottom right corner of the screen -> vertical_shift() 
        if((get_screen_x()==79 && get_screen_y()==24)){                 
            vertical_shift();
            enable_irq(KEYBOARD_IRQ_NUM);   
            return;
        }
        
        // If we are at the end of a line, move onto the next line by printing \n 
        // NOTE: only 80 chars can fit on one row in our terminal when typing 
        if(get_screen_y()<24 && get_screen_x()==79){ 
            printf("%c",'\n');
            enable_irq(KEYBOARD_IRQ_NUM);   
            return;
        }
        
    }
   
    // Re-enable interrupts and send EOI to PIC for IRQ1
    enable_irq(KEYBOARD_IRQ_NUM);             
}


/* 
 * set_pos(int terminal_num_t)
 *   DESCRIPTION: Updates and saves the cursor position for a specified input terminal   
 *   INPUTS: terminal_num_t - number of the terminal we want to save the cursor position for   
 *   OUTPUTS: N/A 
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: N/A
 */
void set_pos(int terminal_num_t){
    update_cursor(get_screen_x(), get_screen_y());                      // Puts the cursor at the current screen_x, screen_y location for the input terminal 
    terminal_num = terminal_num_t;                                      // Set the currently seen terminal to the input terminal 
    set_screen_x(all_terminal_data[terminal_num_t-1].cursor[0]);        // Save the new cursor position for the specified terminal in the terminal's terminal_struct 
    set_screen_y(all_terminal_data[terminal_num_t-1].cursor[1]);
    return;
}




/*KEYBOARD TEST CASES*/
/*working*/
// CAPON:
// `1234567890-=
// QWERTYUIOP[]\//
// ASDFGHJKL;'
// ZXCVBNM,./

/*working*/
// CAPOFF:
// `1234567890-=
// qwertyuiop[]\//
// asdfghjkl;'
// zxcvbnm,./

// CAP+Shift
// ~!@#$%^&*()_+
// qwertyuiop{}|
// asdfghjkl:"
// zxcvbnm<>?

/*working*/
// NO_CAP+Shift
// ~!@#$%^&*()_+
// QWERTYUIOP{}|
// ASDFGHJKL"
// ZXCVBNM<>?

