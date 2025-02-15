#ifndef IDT_H
#define IDT_H

#ifndef ASM

/* Function which initializes the IDT */
extern void idt_init();

/* Handler which handles all interrupts within the IDT */
void handler(int vector);

#endif /* ASM */

#endif


