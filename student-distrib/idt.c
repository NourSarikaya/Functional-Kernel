#include "idt.h"
#include "pit.h"
#include "x86_desc.h"
#include "expcall.h"
#include "syscall_handler.h"
#include "syscall.h"
#include "rtc.h"

#include "lib.h"

#include "i8259.h"
#include "keyboard.h"
#include "rtc.h"


                         
//to get to uppercase of things, offset of 0x59
/*
*   idt_init()
*
*   DESCRIPTION/FUNCTIONALITY: 
*   This function is a initializer for all exception/error/system calls/rtc interrrupt/keyboard interrupt. It initializes every one of 
*   these exceptions, hardware interrupts and system calls.
*
*   INPUTS:   
*   vector        - vector value of the interrupt located within the IDT
*
*   OUTPUTS: 
*   N/a
*/
void idt_init(){

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////// EXCEPTIONS ///////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        int i;
        for(i=0; i<20; i++){

            /*Interrupt gate*/
            if(i==2){
                /* Non-maskable Interrupt (0x2)*/
                idt[i].seg_selector = KERNEL_CS;                // Needs to point to Kernel Code 
                idt[i].reserved4    = 0x00;
                idt[i].reserved3    = 0;                        // Reserved 1,2,3 define gate type (16b or 32b trap/interrupt) (Interrupt=1110)
                idt[i].reserved2    = 1;                        
                idt[i].reserved1    = 1; 
                idt[i].size         = 1;                        // Size = 1 means 32 bits, Size = 0 means 16 bits (size of gate)
                idt[i].reserved0    = 0;                        // Fixed to 0
                idt[i].dpl          = 0;                        // 0 for exception to prevent user level applications from calling into these routines with the int instruction
                idt[i].present      = 1;                        // 1 if a table is filled
            }
            /*Reserved*/
            else if( i==15){ //15 is the index of the Reserved Exception
                //Do Nothing
            }
            /*Trap gate*/
            else{
                idt[i].seg_selector = KERNEL_CS;                // Needs to point to Kernel Code Segment 
                idt[i].reserved4    = 0x00;
                idt[i].reserved3    = 1;                        // Reserved 1,2,3 define gate type (16b or 32b trap/interrupt) (Trap=1111)
                idt[i].reserved2    = 1;                        
                idt[i].reserved1    = 1; 
                idt[i].size         = 1;                        // Size = 1 means 32 bits, Size = 0 means 16 bits (size of gate)
                idt[i].reserved0    = 0;
                idt[i].dpl          = 0;                        // 0 for exception to prevent user level applications from calling into these routines with the int instruction
                idt[i].present      = 1;                        // 1 if a table is filled
            }
        }

        SET_IDT_ENTRY(idt[0], exp_divide_by_zero);       // Assembly linkage to function which saves registers and executes the interrupt handler 
        SET_IDT_ENTRY(idt[1], exp_debug);
        SET_IDT_ENTRY(idt[2], exp_non_mask_interr);
        SET_IDT_ENTRY(idt[3], exp_breakpoint);
        SET_IDT_ENTRY(idt[4], exp_overflow);
        SET_IDT_ENTRY(idt[5], exp_bound_range_exceeded);
        SET_IDT_ENTRY(idt[6], exp_invalid_op);
        SET_IDT_ENTRY(idt[7], exp_device_unavail);
        SET_IDT_ENTRY(idt[9], exp_coprocessor_seg_overrun);
        SET_IDT_ENTRY(idt[16], exp_x87_floating);
        SET_IDT_ENTRY(idt[18], exp_machine_check);
        SET_IDT_ENTRY(idt[19], exp_SIMD_floating);

        /* Exception Vectors:
        *  Index 0: Division Error
        *  Index 1: Debug
        *  Index 2: Non-Maskable Interrupt
        *  Index 3: Breakpoint
        *  Index 4: Overflow
        *  Index 5: Bound Range Exceeded
        *  Index 6: Invalid Opcode
        *  Index 7: Device Not Available
        *  Index 9: Coprocessor Segment Overrun
        *  Index 16: x87 Floating-Point Exception
        *  Index 18: Machine Check
        *  Index 19: SIMD Floating-Point Exception
        */


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////// ERRORS /////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /* Exception Vectors with error codes:
        *  Index 8:  Double Fault
        *  Index 10: Invalid TSS
        *  Index 11: Segment Not Present
        *  Index 12: Stack-Segment Fault
        *  Index 13: General Protection Fault
        *  Index 14: Page Fault
        *  Index 17: Alignment Check
        */


        SET_IDT_ENTRY(idt[8], exp_double_fault);         // Assembly linkage to function which saves registers and executes the interrupt handler 
        SET_IDT_ENTRY(idt[10], exp_invalid_tss);
        SET_IDT_ENTRY(idt[11], exp_seg_not_present);
        SET_IDT_ENTRY(idt[12], exp_stack_seg_fault);
        SET_IDT_ENTRY(idt[13], exp_general_protection_fault);
        SET_IDT_ENTRY(idt[14], exp_page_fault);
        SET_IDT_ENTRY(idt[17], exp_align_check);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////// PIT ///////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /* PIT (0x20)*/
        idt[0x20].seg_selector = KERNEL_CS;                 
        idt[0x20].reserved4    = 0x00;
        idt[0x20].reserved3    = 0;                           // Reserved 1,2,3 define gate type (16b or 32b trap/interrupt)(Interrupt=1110)
        idt[0x20].reserved2    = 1;
        idt[0x20].reserved1    = 1; 
        idt[0x20].size         = 1;
        idt[0x20].reserved0    = 0;
        idt[0x20].dpl          = 0;                           //0 for hardware interrupt to prevent user level applications from calling into these routines with the int instruction
        idt[0x20].present      = 1; 

        SET_IDT_ENTRY(idt[0x20], pit_interrupt);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////// RTC INTERRUPT /////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



        /* RTC Interrupt (0x28) */
        idt[0x28].seg_selector = KERNEL_CS;                // Needs to point to Kernel Code Segment 
        idt[0x28].reserved4    = 0x00;
        idt[0x28].reserved3    = 0;                        // Reserved 1,2,3 define gate type (16b or 32b trap/interrupt)(Interrupt=1110)
        idt[0x28].reserved2    = 1;                        
        idt[0x28].reserved1    = 1; 
        idt[0x28].size         = 1;                        // Size = 1 means 32 bit gate size
        idt[0x28].reserved0    = 0;
        idt[0x28].dpl          = 0;                        // 0 for hardware interrupt to prevent user level applications from calling into these routines with the int instruction
        idt[0x28].present      = 1;
        
        SET_IDT_ENTRY(idt[0x28], rtc_interrupt);            

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////// KEYBOARD //////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        /* Keyboard (33)*/
        idt[33].seg_selector = KERNEL_CS;
        idt[33].reserved4    = 0x00;
        idt[33].reserved3    = 0;                           // Reserved 1,2,3 define gate type (16b or 32b trap/interrupt)(Interrupt=1110)
        idt[33].reserved2    = 1;
        idt[33].reserved1    = 1; 
        idt[33].size         = 1;
        idt[33].reserved0    = 0;
        idt[33].dpl          = 0;                           //0 for hardware interrupt to prevent user level applications from calling into these routines with the int instruction
        idt[33].present      = 1; 

        SET_IDT_ENTRY(idt[33], keyboard_handler);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////// SYSTEM CALLS //////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /* System Calls (0x80) */
        idt[0x80].seg_selector = KERNEL_CS;                // Needs to point to Kernel Code Segment 
        idt[0x80].reserved4    = 0x00;
        idt[0x80].reserved3    = 1;                        // Reserved 1,2,3 define gate type (16b or 32b trap/interrupt)
        idt[0x80].reserved2    = 1;                        
        idt[0x80].reserved1    = 1; 
        idt[0x80].size         = 1;                        
        idt[0x80].reserved0    = 0;
        idt[0x80].dpl          = 3;                        // 3 for system call so that it is accessible from user space via the int instruction
        idt[0x80].present      = 1;

        SET_IDT_ENTRY(idt[0x80], syscall_handler);             

}



/*
*   handler(int vector)
*
*   DESCRIPTION/FUNCTIONALITY: 
*   This function is a handler for all exception/error/system calls. It takes the IDT vector, and executes whichever print statement that corresponds to the interrupt.
*   This print statement is printed to the terminal, and the systems halts after this such that no other operations can be performed. 
*
*   INPUTS:   
*   vector        - vector value of the interrupt located within the IDT
*
*   OUTPUTS: 
*   N/a
*/
void handler(int vector){
    //disable interrupts
    cli();      

    //identify which interrupt was raised, and print this to the terminal 
    //Refer to the IDT Vector tables given above

    /* EXCEPTIONS */                             
    if(vector == 0){
        printf("EXCEPTION: Divide By 0!\n");
    }
    else if(vector == 1){
        printf("EXCEPTION: Debug Error!\n");
    }
    else if(vector == 2){
        printf("EXCEPTION: Non-maskable Interrupt!\n");
    }
    else if(vector == 3){
        printf("EXCEPTION: Breakpoint!\n");
    }
    else if(vector == 4){
        printf("EXCEPTION: Overflow!\n");
    }
    else if(vector == 5){
        printf("EXCEPTION: Bound Range Exceeded!\n");
    }
    else if(vector == 6){
        printf("EXCEPTION: Invalid Opcode!\n");
    }
    else if(vector == 7){
        printf("EXCEPTION: Device Not Available!\n");
    }
    else if(vector == 9){
        printf("EXCEPTION: Coprocessor Segment Overrun!\n");
    }
    /*15 reserved - skipping it*/
    else if(vector == 16){
        printf("EXCEPTION: x87 Floating-Point Exception!\n");
    }
    else if(vector == 18){
        printf("EXCEPTION: Machine Check!\n");
    }
    else if(vector == 19){
        printf("EXCEPTION: SIMD Floating-Point Exception!\n");
    }


    /* ERROR CODES */
    else if(vector == 8){
        printf("EXCEPTION: Double Fault!\n");
    }
    else if(vector == 10){
        printf("EXCEPTION: Invalid TSS!\n");
    }
    else if(vector == 11){
        printf("EXCEPTION: Segment Not Present!\n");
    }
    else if(vector == 12){
        printf("EXCEPTION: Stack Segment Fault!\n");
    }
    else if(vector == 13){
        printf("EXCEPTION: General Protection Fault!\n");
    }
    else if(vector == 14){
        printf("EXCEPTION: Page Fault!\n");
    }
    else if(vector == 17){
        printf("EXCEPTION: Alignment Check!\n");
    }

    /* PIT INTERRUPT */
    else if (vector == 0x20){                              /* RTC interrupt index 0x28 on IDT*/
        pit_handler();                                    //call rtc_handler
    }


    /* RTC INTERRUPT */
    else if (vector == 0x28){                              /* RTC interrupt index 0x28 on IDT*/
        rtc_handler();                                    //call rtc_handler
    }

    /* KEYBOARD */
    else if(vector == 33){                                 /* Keyboard index 33 on IDT*/
        key_handler();
    }
     
    /* SYSTEM CALLS */
    else if(vector == 0x80){                                /* System call index 0x80 on IDT*/
        printf("SYSTEM_CALL!\n");
    }


    //enable interrupts 
    sti();          
    
    //stop the OS from performing any other operations after an interrupt from the IDT is recieved. 
    uint8_t exc_status = (uint8_t) 256;
    
    if(vector < 20){                                    //Exceptions and errors till index 20
        system_halt(exc_status);                        // Squashing exceptions
    }
}
