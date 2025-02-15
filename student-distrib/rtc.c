
/* rtc.c - Initializes RTC interrupts */


#include "rtc.h"
#include "lib.h"
#include "syscall.h"

static int rtc_flag=0;                                  // Set the flag to 1 when an interrupt happens


/* 
 * rtc_init()
 *   DESCRIPTION: Enables/Disables periodic interrupts from the RTC depending on the RTC_ON macro from tests.h. If RTC_ON = 1, enable RTC interrupts. If RTC_ON = 0, disable 
 *                RTC interrupts.   
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets the enable_periodic_interrupts bit in Register B to 1 or 0 (depending on RTC_ON). Links RTC interrupts to IRQ8 on the PIC.
 *   NOTES: When accessing Register B, use outb(0x8B, RTC_INDEX_PORT). 0x8B tells the RTC chip that we want to access the register at index 0x0B. By setting the highest bit of this
 *          to 1 (0x8B), this disables NMI interrupts as we read/write to/from Register B. 
 */ 
void rtc_init(){
    
    
    cli();                                              // Disable interrupts 
     
    
    if(RTC_ON == 1){                                    // Enable/Disable RTC Interrupts depending on the value of RTC_ON in tests.h
        outb(0x8B, RTC_INDEX_PORT);                     // Disable NMI, Read from Register B 
        char old_B = inb(RTC_DATA_PORT);        
        outb(0x8B, RTC_INDEX_PORT);                     // Write to Register B   
        outb(old_B | 0x40, RTC_DATA_PORT);              // Enable periodic interrupts by setting the enable_periodic_interrupt bit = 1 (ORing with 0x40 = 0100 0000 = sets enable flag)
        enable_irq(RTC_IRQ_NUM); 
    }
    else{
        outb(0x8B, RTC_INDEX_PORT);                     // Disable NMI, Read from Register B 
        char old_B = inb(RTC_DATA_PORT);    
        outb(0x8B, RTC_INDEX_PORT);                     // Write to Register B
        outb(old_B & 0xBF, RTC_DATA_PORT);              // Disable periodic interrupts by setting the enable_periodic_interrupt bit = 0 (ANDing with 0xBF = 1011 1111 = enable flag is 0)
        enable_irq(RTC_IRQ_NUM);
    }
    
    
    sti();                                              // Enable Interrupts 
}


/* 
 * rtc_handler()
 *   DESCRIPTION: Handler for RTC Interrupts. Calls the test_interrupts() routine provided in lib.c every time an RTC interrupt is raised.    
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Reads Register C such that we are able to recieve more than one RTC Interrupt. 
 */ 
void rtc_handler(){
    rtc_flag=1;

    mask_and_ack(RTC_IRQ_NUM);           // Mask RTC Interrupts

    outb(0x0C, RTC_INDEX_PORT);          // Read Register C and do nothing with this information 
    inb(RTC_DATA_PORT);
    //printf(" RTC Interrupt!\n");
 
    //test_interrupts();                 // Call routine to handle interrupt 
    
    enable_irq(RTC_IRQ_NUM);             // Enable RTC Interrupts
}


/* 
 * rtc_open
 *   DESCRIPTION: Resets the RTC frequency to 2Hz.   
 *   INPUTS: filename - filename that corresponds to a a directory entry
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on successfull execution
 *   SIDE EFFECTS: Sets the last 4 bits of Status Register A to the rate for which frequency=2Hz.
 *
 *   Refer to:  https://wiki.osdev.org/RTC#Changing_Interrupt_Rate
 */ 
int32_t rtc_open(const uint8_t* filename){
    dentry_t dentry;

    uint8_t rate = 0x01;                        // Initialize the rate to 0x01
    int32_t frequency=2;                        //  Set the frequency to 2Hz
    //rate=15 -> frequency= 2Hz
    while((32768 >> (rate-1)) > frequency){     // The formula to calculate the interrupt rate: frequency =  32768 >> (rate-1) where 32768 is the default frequency
        rate++;                                 // 0x8000 , original rate in rtc, any frequency is this number divide by some power of 2
    }
    rate &= 0x0F;	                            // rate is the lower 4 bits of status register A

    cli();                                      // Disable interrupts 

    outb(0x8A, RTC_INDEX_PORT);                 // Disable NMI, Read from Register A
    uint8_t prev = inb(RTC_DATA_PORT);          // get initial value of register A
    outb(0x8A, RTC_INDEX_PORT);                 // reset index to A
    outb((prev & 0xF0) | rate, RTC_DATA_PORT);  // write only our rate to A. Note, rate is the bottom 4 bits.

    
    int retval = read_dentry_by_name (filename, &dentry);
    // check if file type is 0, which is rtc
    if (retval != 0 || dentry.file_type!=0) {
        /*return fail if file type is not a rtc*/
        return -1;
    }
    int i; 
    // 2 as stdin and stdout is the first 2, 8 as the array length is 8
    for(i=2; i<8; i++){         
        if(pcb_ptr()->fds[i].flags==0){
            //ignore index node number and file position as we are dealing with rtc
            pcb_ptr()->fds[i].inode_num = 0;
            pcb_ptr()->fds[i].file_pos = 0; 
            pcb_ptr()->fds[i].flags = 1;        //Mark rtc file as in use
            return i; 
        }
    }
    sti();                                      // Enable Interrupts 

    return -1;
}


/* 
 * rtc_read
 *   DESCRIPTION: Returns after an RTC interrupt has occurred otherwise waits.(rtc_flag is set to 1 in the rtc_handler)   
 *   INPUTS: fd - N/a
 *           buf - N/a
 *           nbytes- N/a
 *
 *   OUTPUTS: none
 *   RETURN VALUE: 0 after an RTC interrupt has occurred
 *   SIDE EFFECTS: none
 */ 
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes){

    while(rtc_flag==0){                         // Wait for an RTC interrupt 
        continue;
    }
    //printf("rtc_read: Interrupt fired!\n");
    rtc_flag=0;                                 // Set the RTC flag back to 0 
    return 0;
}


/* 
 * rtc_write
 *   DESCRIPTION: Changes the rtc frequency.
 *   INPUTS: fd - N/a
 *           buf - teh buffer pointer that points to the frequency
 *           nbytes- N/a
 *
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on successfull execution
 *                 -1 when frequency is invalid
 *   SIDE EFFECTS: Sets the last 4 bits of Status Register A to the rate (depending on frequency).
 */ 
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    int32_t frequency= *(int32_t*) buf;

    if((frequency & (frequency - 1)) != 0){     // Formula to check if a number is a power of two: (freq & (freq - 1)) == 0
        return -1;                              // If not a power of 2 then return -1
    }


    uint8_t rate = 0x01;                        // Initialize the rate to 0x01

  
    while((32768 >> (rate-1)) > frequency){     // The formula to calculate the interrupt rate: frequency =  32768 >> (rate-1) where 32768 is the default frequency
        rate++;                                 // 0x8000 , original rate in rtc, any frequency is this number divide by some power of 2
    }

    
    
    if(rate > 15){                              // rate must be below 15(corresponding freq=2 Hz) 
        return -1;
    } 
    else if(rate < 6){                          // rate must be on or above 6(corresponding freq=1024 Hz) because an operating system shouldn’t allow userspace programs to 
        return -1;                              // generate more than 1024 interrupts per second by default.
    }

    
    rate &= 0x0F;	                            // rate is the lower 4 bit of status register A (0x0F is the mask to get the lower 4 bits)


    //Possible frequencies:
    //rate=6 -> frequency= 1024Hz
    //rate=7 -> frequency= 512Hz
    //rate=8 -> frequency= 256Hz
    //rate=9 -> frequency= 128Hz
    //rate=10 -> frequency= 64Hz
    //rate=11 -> frequency= 32Hz
    //rate=12 -> frequency= 16Hz
    //rate=13 -> frequency= 8Hz
    //rate=14 -> frequency= 4Hz
    //rate=15 -> frequency= 2Hz	

    cli();                                      // Disable Interrupts

    outb( 0x8A ,  RTC_INDEX_PORT);		        // set index to register A, disable NMI
    uint8_t prev = inb(RTC_DATA_PORT);	        // get initial value of register A
    outb( 0x8A ,  RTC_INDEX_PORT);	            // reset index to A
    outb((prev & 0xF0) | rate , RTC_DATA_PORT); // write only our rate to A. Note, rate is the bottom 4 bits.

   
    sti();                                      // Enable Interrupts

    return nbytes;

   
    
}


/* 
 * rtc_close
 *   DESCRIPTION: Currently does not do anything and returns a constant value
 *   INPUTS: fd - N/a
 *   OUTPUTS: none
 *   RETURN VALUE: 0 always
 *   SIDE EFFECTS: nopne
 */ 
int32_t rtc_close(int32_t fd){
    /*check if the fd array has fd's flag that's on*/
    if(pcb_ptr()->fds[fd].flags !=0 ){
        /*sets the flag to 0 to close the files*/
        pcb_ptr()->fds[fd].flags = 0;
        /*return success*/
        return 0; 
    }

    /*return -1 if falied to close*/
    return -1; 
}

