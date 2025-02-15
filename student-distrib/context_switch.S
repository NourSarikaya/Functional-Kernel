
#define ASM 1

#
#   context_switch
#
#   DESCRIPTION/FUNCTIONALITY: 
#   This function store all general registers with esp, eip, eflags, ebp inside the given pcb  
#   After that, this function set up the stack according to the given input and call iret to perform
#   context switch
#
#   INPUTS:   
#   pcb - the pcb to store the data to
#   eip - the desired eip to jump to
#   cs - the desired code selector after the context switch
#   eflags - the desired eflags after the context switch
#   esp - the desired esp after the context switch 
#   ss - the desired stack selector after the context switch
#   
#   REGISTERS: 
#   ecx - temporary register used to store arguements
#   ebx - temporary register used to store arguements
#
#   OUTPUTS: 
#   N/a
#
.text
.globl context_switch
context_switch: 
    
    movl 4(%esp), %ecx    # store pcb inside ecx

    # store data into pcb
    # see the pcb_t struct in syscall.h for the explanation of numbers below
	pushfl				        # push eflags onto stack
	movl	%ebp, 0x0(%ecx)		# save EBP in suspd->ebp
	movl	%eax, 0x4(%ecx)	    # save EAX in suspd->eax
	movl	%ebx, 0x8(%ecx)	    # save EBX in suspd->ebx
	movl	%ecx, 0xC(%ecx)	    # save ECX in suspd->ecx
	movl	%edx, 0x10(%ecx)	# save EDX in suspd->edx
	movl	%esi, 0x14(%ecx)	# save ESI in suspd->esi
	movl	%edi, 0x18(%ecx)	# save EDI in suspd->edi
	popl 	0x1C(%ecx)		    # save EFLAGS in suspd->elfags
	popl	0x20(%ecx)		    # save EIP in suspd->eip
	#movl	%esp, 0x24(%ecx)	# save ESP in suspd->esp

    # dont need the below code as c do the caller clean up of the stack automatically
    popl    %ebx                # pop out pcb as we are done with it
    movl    %esp, %ebx          # temp store esp into ebx
    add     $-20, %ebx          # minus 20 as there are 5 arguements left on the stack, each occupying 4 bytes
	movl	%ebx, 0x24(%ecx)	# save ESP in suspd->esp

    # dont touch the stack for above mentioned reason
    popl    %ebx                # store destination eip into ebx
    popl    %ecx                # store destination cs into ecx
    popl    %edx                # edx is not used, just need an arguement or else compiler screams

    # setup stack
    #movl    4(%esp),%ebx       # store destination eip into ebx
    #movl    8(%esp),%ecx       # store destination cs into ecx
    #movl    12(%esp),%edx      # store destination eip into ebx
    #movl    16(%esp),%esi      # store destination cs into ecx
    #pushl   %esi               # push back ss onto stack
    #pushl   %edx               # push back esp onto stack
    pushfl
    pushl   %ecx                # push back cs onto stack
    pushl   %ebx                # push back eip onto stack

    iret                        # call iret to perform context switch


#
#   restore_reg
#
#   DESCRIPTION/FUNCTIONALITY: 
#   This function restore all general registers with esp, eip, eflags, ebp from the given pcb  
#   After that, this function set up the stack according to the stored eip ret to 
#   jump to system_execute right after we call context_switch
#
#   INPUTS:   
#   pcb - the pcb to restore the data from
#   
#   REGISTERS: 
#   ecx - temporary register used to store arguements
#   edx - temporary register used to store arguements
#   eax - restore the value from the given pcb
#   ebx - restore the value from the given pcb
#   esi - restore the value from the given pcb
#   edi - restore the value from the given pcb
#   ebp - restore the value from the given pcb
#   esp - restore the value from the given pcb
#   eip - restore the value from the given pcb
#
#   OUTPUTS: 
#   N/a
#
.text
.globl restore_reg
restore_reg: 
    
    popl    %ecx                # pop out origin return addr as we are jumping back to execute
    movl    (%esp), %ecx        # store pcb inside ecx

    # restore data from pcb
    # see the pcb_t struct in syscall.h for the explanation of numbers below
	movl	0x4(%ecx), %eax	    # restore EAX from suspd->eax
	movl	0x8(%ecx), %ebx	    # restore EBX from suspd->ebx
	movl	0x10(%ecx), %edx	# restore EDX from suspd->edx
	movl	0x14(%ecx), %esi	# restore ESI from suspd->esi
	movl	0x18(%ecx), %edi	# restore EDI from suspd->edi

	pushl 	0x1C(%ecx)	        # restore EFLAGS from suspd->elfags
    popfl 

    popl    %edx                # pop out the arguement pcb (clean the stack)

    movl	0x0(%ecx), %ebp		# restore EBP from suspd->ebp
    movl	0x24(%ecx), %esp	# restore ESP from suspd->esp
    pushl   $0                  # push 6 zeros because we have 6 arguments - we pop these arguments off, and C also pops these arguments off. Pushing these 6 arguments allows the stack to not be messed up. 
    pushl   $0
    pushl   $0
    pushl   $0
    pushl   $0
    pushl   $0
	pushl	0x20(%ecx)		    # push EIP from suspd->eip onto the stack

    ret                         # call ret to switch back to the end of execute















