#include "pit.h"


/* Static Variables to Keep Track of Scheduling States/Milestones */
static int terminal_being_handled = -1; 															// terminal currently being handled in scheduling
static int counter=-1;																				// counter which gets incremented inside of pit_handler()


/* 
 * pit_init()
 *   DESCRIPTION: Initializes the pit to interrupt at a frequency of 20Hz = 50ms.  
 *   INPUTS: none 
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: The pit enters the pit_handler every 50ms. 
 *   LINK: https://wiki.osdev.org/Programmable_Interval_Timer
 */
void pit_init(){

    cli();																							// Disable Interrupts 
	int Hz=20;																						// Set pit interrupt rate to 20Hz = 50ms 
	int divisor =1193180/Hz;																		// 1193180 Hz = input clock rate of the pit 
	outb(0x00110100b, MODE_COMMAND_REG);															// 0x00110100b: Select Channel 0, Set Access Mode to lobyte/hibyte, Set the mode to mode 2 (rate generator)
	outb(divisor&0xFF,CHANNEL0);																	// Set the low byte (divisor&0xFF:selecting low byte) of the divisor 
    outb(divisor>>8,CHANNEL0);     																	// Set the high byte (divisor>>8:selecting high byte) of the divisor 
    sti();
    enable_irq(PIT_IRQ_NUM);

}


/* 
 * get_terminal_being_handled()
 *   DESCRIPTION: Returns the terminal currently being handled in scheduling   
 *   INPUTS: none 
 *   OUTPUTS: none 
 *   RETURN VALUE: The terminal currently being handled in scheduling   
 *   SIDE EFFECTS: none  
 */
int get_terminal_being_handled(){
	return terminal_being_handled;
}


/* 
 * get_counter()
 *   DESCRIPTION: Returns the counter that is incremented inside of pit_handler().    
 *   INPUTS: none 
 *   OUTPUTS: none 
 *   RETURN VALUE: The counter that is incremented inside of pit_handler(). 
 *   SIDE EFFECTS: none  
 */
int get_counter(){
	return counter; 
}


/* 
 * pit_handler()
 *   DESCRIPTION: Handler that deals with pit interrupts. On the first three interrupts, the 3 base shells for terminals 1, 2, and 3 are spawned. On the 
 *				  fourth interrupt, the first terminal is set as the "seen" terminal. The fourth and fifth interrupts are simply waiting periods to allow 
 *				  any processes from the prior interrupts to finish. After this waiting period, scheduling is activated. 
 * 
 * 			      Note that the terminal being handled for scheduling is based off of the counter that is incremented every time the pit_handler() is entered.
 *				  This value rotates between terminals 1, 2, and 3 in a cyclic pattern. This value is then passed into our scheduler function. 
 *   INPUTS: none 
 *   OUTPUTS: none 
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Spawns the 3 base shells and sets up the round-robin for scheduling.   
 */
void pit_handler(){
	
	counter+=1;																					// Increment the pit counter 

	terminal_being_handled = (counter%3)+1;  													// Calculate the terminal being handled for scheduling (1 index => we do +1, there are 3 terminal so we do %3 of the counter)
    mask_and_ack(PIT_IRQ_NUM);
	
	if(counter==0){																				/* Initialize Terminal 1's Shell */

		clear();
		enable_irq(PIT_IRQ_NUM); 
		set_pos(1);																				// Update the cursor so it corresponds with terminal 1's position
		set_seen_terminal_num(1);																// Set the "seen" terminal to terminal 1
		terminal_data(1)->last_run_pcb = (pcb_t*)(0x800000 - (0x2000 * 1)); 					// Set the last_run_pcb for terminal 1 to point towards the PCB for shell 1 (8MB - 8KB) 
		load_4MB_syscall_page(0);																// Map program image to 1st terminal's base shell
		while(1)
			system_execute((const uint8_t*)("shell"));
	}
	else if(counter==1){																		/* Initialize Terminal 2's Shell */

		copy_terminal_data(2);																	// Map video memory to terminal 2's page 
		enable_irq(PIT_IRQ_NUM); 
		set_pos(2);																				// Update the cursor so it corresponds with terminal 2's position
		set_seen_terminal_num(2);																// Set the "seen" terminal to terminal 2
		terminal_data(2)->last_run_pcb = (pcb_t*)(0x800000 - (0x2000 * 2)); 					// Set the last_run_pcb for terminal 2 to point towards the PCB for shell 2 (8MB - 2*8KB)
		load_4MB_syscall_page(1);																// Map program image to 2nd terminal's base shell
		while(1)
			system_execute((const uint8_t*)("shell"));
	}
	else if(counter==2){																		/* Initialize Terminal 3's Shell */

		copy_terminal_data(3);																	// Map video memory to terminal 3's page 
		enable_irq(PIT_IRQ_NUM); 
		set_pos(3);																				// Update the cursor so it corresponds with terminal 3's position
		set_seen_terminal_num(3); 																// Set the "seen" terminal to terminal 3
		terminal_data(3)->last_run_pcb = (pcb_t*)(0x800000 - (0x2000 * 3)); 					// Set the last_run_pcb for terminal 3 to point towards the PCB for shell 3 (8MB - 3*8KB)
		load_4MB_syscall_page(2);																// Map program image to 3rd terminal's base shell
		while(1)
			system_execute((const uint8_t*)("shell")); 
	}
	else if(counter==3){																		/* Start in Terminal 1 */

		copy_terminal_data(0);																	// Map video memory to physical video memory 
		set_pos(1);																				// Update the cursor so it corresponds with terminal 1's position
		change_global_pcb(((terminal_t*)(terminal_data(1)))->last_run_pcb);						// Change the global pcb in syscall.c to point to the PCB of terminal 1's base shell (8MB - 8KB)
		set_seen_terminal_num(1);																// Set the "seen" terminal to terminal 1
		load_4MB_syscall_page(0); 																// Map the Process Image to the code for Terminal 1's shell 
	}else if(counter == 4){
		// Pass 
	} else if(counter == 5){
		// Pass 
	}
	else{																						/* Start Scheduling */
		enable_irq(PIT_IRQ_NUM);
		sti();					
		//schedule(terminal_being_handled);
		return;
	}
	

    enable_irq(PIT_IRQ_NUM);

}
