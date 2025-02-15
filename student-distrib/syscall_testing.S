#define ASM 1

# File that contains a test function for the execute system call. This function loads the ebx register with the command,
# and calls int80 to go through the IDT and Syscall handler. 
#
# int32_t syscall_parse (const int num, const uint8_t* command);
#
#       Interface: Arguments are pushed onto stack, int80 is called to enter Syscall Handler. 
#
#          Inputs: command - Name of executable desired to be run
#                  num - number (0,1,2,3) that specifies which syscall we are doing  
#                        {1 = halt, 2 = exdcute, etc.}
#           
#        Function: A test function for the execute system call. This function loads the ebx register with the command,
#                  and calls int80 to go through the IDT and Syscall handler.  
#
#          Output: A part of the testing path to execute a valid executable/  
#
#       Registers: %ebx - holds command argument  
#                  %eax - holds system call identification number 
# 
.text
.globl syscall_parse
syscall_parse: 
    push %ebp                           # Stack Setup 
    movl %esp, %ebp
    movl 8(%esp), %eax                  # Store syscall ID # in EAX 
    movl 12(%esp), %ebx                 # Store command in EBX
    int $0x80
    leave
    ret

