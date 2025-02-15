#define ASM     1


#
#   loadPageDirectory
#
#   DESCRIPTION/FUNCTIONALITY: 
#   This function loads the base address of the Page Directory into register CR3, where 
#   CR3 will only ever hold this address (and nothing else). 
#
#   INPUTS:   
#   page_directory_base_addr - the base address of the Page Directory 
#   
#   REGISTERS: 
#   eax - holds Page Directory base address
#   cr3 - stores the Page Directory base address 
#
#   OUTPUTS: 
#   N/a
#
.text
.globl loadPageDirectory
loadPageDirectory:
push %ebp                       # Stack Setup 
mov %esp, %ebp
mov 8(%esp), %eax               # eax <- [Page Directory Base Address]
mov %eax, %cr3                  # cr3 <- eax
leave
ret                             # Stack Teardown and Return 





#
#   enablePaging
#
#   DESCRIPTION/FUNCTIONALITY: 
#   This function enables paging by setting the Page Size Extension bit in CR4 to 1, the Enable 
#   Paging Bit in CR0 to 1, and the Enable Protected Mode Bit in CR0 to 1. All of these modes must 
#   be enabled to have paging be used.  
#
#   INPUTS:   
#   N/a 
#   
#   REGISTERS: 
#   eax - acts as a temporary register 
#   cr4 - register which masks certain OS modes (https://wiki.osdev.org/CPU_Registers_x86#CR4)
#   cr0 - register which masks certain OS modes (https://wiki.osdev.org/CPU_Registers_x86#CR0)
#
#   OUTPUTS: 
#   N/a
#
.globl enablePaging
enablePaging:
push %ebp                       # Stack Setup 
mov %esp, %ebp
mov %cr4, %eax                  
or $0x010, %eax                 # cr4[bit 4] = 1  (enables Page Size Extension)
mov %eax, %cr4                  # such that pages >4kB in size 
mov %cr0, %eax
or  $0x80000001, %eax           # cr0[bit 32] = 1  (enables paging)  
mov %eax, %cr0                  # and cr0[bit 0] = 1  (enables protected mode - virtual memory, paging, etc.) 
leave
ret                             # Stack Teardown and Return 

#
#   tlb_flush
#
#   DESCRIPTION/FUNCTIONALITY: 
#   This function flush the tlb by reloading the value of cr3 
#
#   INPUTS:   
#   N/a 
#   
#   REGISTERS: 
#   eax - acts as a temporary register 
#   cr3 - base page address register
#
#   OUTPUTS: 
#   N/a
#
.globl tlb_flush
tlb_flush:
mov %cr3, %eax                  # reset tlb
mov %eax, %cr3                  # 
ret                             # Stack Teardown and Return 

