/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 *  Research about PIC extracted from: https://wiki.osdev.org/8259_PIC
 */

#include "i8259.h"
#include "lib.h"


/* 
 * i8259_init
 *   DESCRIPTION: Initialize the 8259 PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the PIC by sending ICW1 - ICW4 data to both the master and the slave. 
 */
void i8259_init(void) {

    unsigned long flags;
    cli_and_save(flags);

	outb(0xFF, PIC1_DATA);                              // mask all interrupt as it is the device's routine to open it up
	outb(0xFF, PIC2_DATA);

	outb(ICW1, PIC1_COMMAND);                           // ICW1: starts the initialization sequence (in cascade mode)
	outb(ICW1, PIC2_COMMAND);                           // ICW1: starts the initialization sequence (in cascade mode)

	outb(ICW2_MASTER, PIC1_DATA);                       // ICW2: Master PIC vector offset
	outb(ICW2_SLAVE, PIC2_DATA);                        // ICW2: Slave PIC vector offset

	outb(ICW3_MASTER, PIC1_DATA);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (1000 0100)
	outb(ICW3_SLAVE, PIC2_DATA);                        // ICW3: tell Slave PIC its cascade identity (1000 0010)
 
	outb(ICW4, PIC1_DATA);                              // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	outb(ICW4, PIC2_DATA);

/*  ICW4 if we are using auto eoi
	outb_p(PIC1_DATA, ICW4 | ICW4_AUTO);                // ICW4 but in AUTO EOI mode
	outb_p(PIC2_DATA, ICW4 | ICW4_AUTO);
 */
 	outb(0xFF, PIC1_DATA);                              // mask all interrupt again as it is part of the device's
	outb(0xFF, PIC2_DATA);                              // initialization routine to unmask irq

    enable_irq(2);                                      // as the slave PIC is initialized, open up that particular mask (IRQ 2)
    restore_flags(flags);

}


/* 
 * disable_irq
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *   INPUTS: irq_num -- the irq number to be masked, if want to specify the one in slave device, should set to irq number + 8
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: masked the specified irq
 */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
 
    unsigned long flags;
    cli_and_save(flags);                    // should use spinlock instead of cli but currently spinlock is not implemented

    if(irq_num < 8) {                       // Check if the user wants to enable the irq in slave or master
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_num -= 8;                       
    }
    value = inb(port);
    //printf("disabling irq\n");
    //printf("original value is %d \n", value);
    //printf("irq_num is %d \n", irq_num);
    value = value | (1 << irq_num);         // only change that specific mask and leave others untouched
    //printf("value is %d \n", value);
    outb(value, port);
    restore_flags(flags);
}


/* 
 * enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ
 *   INPUTS: irq_num -- the irq number to be unmasked, if want to specify the one in slave device, should set to irq number + 8
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: unmasked the specified irq
 */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;

    unsigned long flags;
    cli_and_save(flags);                        // should use spinlock instead of cli but currently spinlock is not implemented

    if(irq_num < 8) {                           // Check if the user wants to disable the irq in slave or master
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
    }
    value = inb(port);
    //printf("enabling irq\n");
    //printf("original value is %d \n", value);
    //printf("irq_num is %d \n", irq_num);
    value = value & ~(1 << irq_num);
    //printf("value is %d \n", value);
    outb(value, port);
    restore_flags(flags);    
}


/* 
 * send_eoi
 *   DESCRIPTION: send eoi to the pic
 *   INPUTS: irq_num -- indicate wether we are sending EOI to the master or the slave, should set to irq number + 8 if sending to slave PIC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Send end-of-interrupt signal for the specified IRQ
 */
void send_eoi(uint32_t irq_num) {
    //printf("sending EOI with irq_num being %d\n", irq_num);
    unsigned long flags;
    cli_and_save(flags);                                    // should use spinlock instead of cli but currently spinlock is not implemented

    // 8 as only 8 irq in a PIC
	if(irq_num >= 8){
        irq_num -= 8;
		outb((EOI | irq_num), PIC2_COMMAND); 
        outb((EOI | 0x02), PIC1_COMMAND);                   // or the eoi with IR2 (0000 0010) as it is where slave pic is
    }
    else{
        outb((EOI | irq_num), PIC1_COMMAND);
    }
    restore_flags(flags);   
}

/* 
 * mask_and_ack
 *   DESCRIPTION: mask corresponding irq and send eoi
 *   INPUTS: irq_num -- indicate which irq to mask and send doi and wether we are sending EOI to the master or the slave, should set to irq number + 8 if sending to slave PIC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Mask corresponding irq and send end-of-interrupt signal for the specified IRQ
 */
void mask_and_ack(uint32_t irq_num){
    // cannot use the same spinlock in disable_irq/send_eoi as a deadlock will form. CARE!
    // better to lock? not sure if it matters
    disable_irq(irq_num);
    send_eoi(irq_num);
}

