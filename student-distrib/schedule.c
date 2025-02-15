#include "schedule.h"
#include "syscall.h"

/* 
 * schedule()
 *   DESCRIPTION: This function handles each terminal in a round robin fashion. The terminal to be handled is handled by the PIT. To switch to the next terminal to handle,
 *                we do the following: 
 *                  1) Save the last run pcb of the terminal we are leaving 
 *                  2) Calculate the number for the terminal we must switch to 
 *                  3) Aquire our kernel stack ESP and EBP such that later we can position ourselves to be looking at the kernel stack of the next process we want to switch to 
 *                  4) Change the global pcb in syscall.c to be last run pcb of the terminal we are switching to 
 *                  5) Map the program image to the program page for the terminal we are going to
 *                  6) Set the esp0 in TSS based off of the new current terminal's stack 
 *                  7) Map video memory either to the seen screen (if we are servicing the seen terminal) or to the not-seen terminal's page 
 *                  8) Save the ESP and EBP of the terminal we are leaving into that terminal's terminal struct, inside of the last_run_pcb
 *                  9) Put the ESP and EBP values we saved earlier for the terminal we are switching to into the actual ESP and EBP registers
 *   INPUTS: terminal_being_handled - 1 or 2 or 3 for each terminal
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Allows for us to continuously rotate through all of the terminals and service their current processes for 50ms. 
 */
void schedule(int terminal_being_handled){
    // uint32_t new_esp, new_ebp, next_terminal;
    // int i;

    // //Save the last run pcb of the terminal we are leaving 
    // terminal_data(terminal_being_handled)->last_run_pcb = pcb_ptr(); 
    // //

    // // next terminal to switch to
    // next_terminal = (terminal_being_handled % 3) +1;

    // // set esp and ebp to the next terminal process' value
    // new_esp = terminal_data(next_terminal)->last_run_pcb->esp;
    // new_ebp = terminal_data(next_terminal)->last_run_pcb->ebp;

    // // for(i = 1 ; i < 4 ; i++){
    // //     printf("terminal_data(%d)->last_run_pcb->esp is %d\n",i,terminal_data(i)->last_run_pcb->esp);
    // //     printf("terminal_data(%d)->last_run_pcb->ebp is %d\n",i,terminal_data(i)->last_run_pcb->ebp);
    // // }
    
    // // while(1){
    // //     //
    // // }

    // // change the pcb we are accessing to the current process' pcb
    // change_global_pcb(terminal_data(next_terminal)->last_run_pcb);                        

    // // load the corresponding code for the process into program virtual page
    // load_4MB_syscall_page(terminal_data(next_terminal)->last_run_pcb->process_num);

    // // set the esp0 to the esp value of the current terminal's process' esp
    // // plus 1 as process num is 0 based
    // tss.esp0 = 0x800000-(0x2000 * (terminal_data(next_terminal)->last_run_pcb->process_num + 1));  

    // // if the terminal we r handling is the one on the screen
    // // if(next_terminal==get_seen_terminal_num()){
    // //     // map to 0 -> enable printf to show on screen
    // //     copy_terminal_data(0);
    // // }
    // // else{
    // //     // map to buffer -> "background"
    // //     copy_terminal_data(next_terminal);
    // // }  

    // // stored the old esp and ebp
    // asm("movl %%ebp, %0;" : "=r" (terminal_data(terminal_being_handled)->last_run_pcb->ebp):);   // Save current EBP
    // asm("movl %%esp, %0;" : "=r" (terminal_data(terminal_being_handled)->last_run_pcb->esp):);   // Save current ESP 

    // // asm("movl %0, %%ebp;" : :"r" (terminal_data(next_terminal)->last_run_pcb->ebp));   // Save current EBP
    // // asm("movl %0, %%esp;" : :"r" (terminal_data(next_terminal)->last_run_pcb->esp));   // Save current EBP
    // // // assign the new esp and ebp
    // // uint32_t move_to_this_ebp;
    // // uint32_t move_to_this_esp;
    
    // uint32_t a=terminal_data(next_terminal)->last_run_pcb->ebp, b;
    // Move the esp and ebp for the next terminal to service into registers esp and ebp with inline assembly. 
    // asm ("movl %1, %%ebp; 
    //         movl %%ebp, %0;"
    //         :"=r"(b)        /* output */
    //         :"r"(a)         /* input */
    //         :"%ebp"         /* clobbered register */
    //     );       

    // uint32_t c=terminal_data(next_terminal)->last_run_pcb->esp, d;
    // asm ("movl %1, %%esp; 
    //         movl %%esp, %0;"
    //         :"=r"(d)        /* output */
    //         :"r"(c)         /* input */
    //         :"%esp"         /* clobbered register */
    //     );     

}

