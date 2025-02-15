#include "syscall.h"
#include "expcall.h"
#include "paging.h"
#include "filesystem.h"
#include "context_switch.h"
#include "x86_desc.h"
#include "rtc.h"
#include "lib.h"
 
/* HELPER GLOBAL VARIABLES */
static int pcb_array[PCB_ARR_MAX_COUNT]={0};                    // Flag array which ensures only 2 processes are running at once 
static pcb_t* pcb = NULL;                                       // Pointer to the pcb of the current executing process 
static pcb_t* pcb_copy=NULL;
static int num_times_ex_called = 0;                                 // Number of times execute function is entered
static uint8_t arg_buf[128]={'\0'};

/* 
 *   pcb_ptr()
 *   DESCRIPTION: Returns a pointer to the pcb of the process currently executing   
 *   INPUTS: N/A
 *   OUTPUTS: N/A
 *   RETURN VALUE: Pointer to currnent process pcb 
 */
pcb_t* pcb_ptr(){
    return pcb; 
}


/* 
 *   change_global_pcb(pcb_t* addr)
 *   DESCRIPTION: Set the global pcb pointer to point to addr    
 *   INPUTS: addr - what we want the global pcb pointer to be pointing towards 
 *   OUTPUTS: N/A
 *   RETURN VALUE: N/A
 */
void change_global_pcb(pcb_t* addr){
    pcb=addr; 
}


/* 
 *   dummy()
 *   DESCRIPTION: Does nothing: acts as a space filler such that the jump table in syscall_handler.S workers properly.    
 *   INPUTS: N/A
 *   OUTPUTS: N/A
 *   RETURN VALUE: Does nothing - "space filler."
 */
int32_t dummy (){
    return 0;
}


/* 
 *   system_halt (uint8_t status)
 *   DESCRIPTION: Once an executable has reached the end of its execution (or once "return" is reached), system_halt() is called. 
 *                This function tears down the kernel stack previously allocated to the current process, and resumes execution where 
 *                the parent process left off. This function checks to see if the desired process to halt had a parent. If not (so, if we
 *                are trying to halt the first shell), then this process is halted, and then reloaded/re-executed. If this is not the case, 
 *                the process is halted and parent process data is restored.    
 *   INPUTS: status - Data sent from the process informing the OS on how the process halted (exception=256, normal=[0,255]). 
 *   OUTPUTS: n/a
 *   RETURN VALUE: -1 if failed to halt, 0 if successful   
 */
int32_t system_halt (uint8_t status){
    
    int parent_id_temp, i;
    cli();                                                                  // Disable Interrupts 

    deallocate_page(34);                                                    // Set vidmap page's which is at index 34 present bit to 0
    
    if(pcb->process_num < 0 || pcb->process_num > PCB_ARR_MAX_COUNT){       // If the process ID is invalid, DO NOT HALT 
        printf("halting not existing process\n");
        sti();
        return -1;                                                          // Return -1 for failure
    }

    parent_id_temp = pcb->parent_id;           

    if(parent_id_temp == -1){                                               // If the process we want to halt does not have a parent:
        pcb_array[pcb->process_num] = 0;                                    //      - Set process array flag low 
        pcb_copy=NULL; 
        num_times_ex_called = num_times_ex_called - 1;                      
        for(i = 2 ; i < 8 ; i++){                                           //      - Clear pcb FD array
            system_close (i);
        }
        sti();
        tss.ss0 = KERNEL_DS;                                                //      - Set segment selector to KERNEL_DS and the future ESP to 8MB
        tss.esp0 = 0x800000-(0x2000 * (pcb->process_num));                  //      - Set esp0 to point to the next available stack (8MB - 8KB*process-num)
        restore_reg(pcb);                                                   //      - Now, restore the process we just halted
        return 0;
    }

    num_times_ex_called = num_times_ex_called - 1;                          

    pcb_array[pcb->process_num] = 0;                                        // Set the process flag low 

    for(i = 2 ; i < 8 ; i++){                                               // Clear pcb FD array
        system_close(i);
    }

    sti(); 
    int curr_terminal = pcb->terminal;

    pcb_t* parent = (pcb_t*) (0x800000 - (0x2000 * (parent_id_temp+1)));    // Gain access to the parent pcb 
    load_4MB_syscall_page(parent_id_temp);                                  // Associated parent physical memory with virtual memory page 
    pcb_copy=pcb;
    pcb_copy->halt_status=status;                                           // Store the exit status of the current process in its pcb 
    pcb=parent;                                                             // Set the parent process as the current process 

    terminal_t* current_terminal = terminal_data(curr_terminal);            // Update the last run pcb for the terminal that we are currently in
    current_terminal->last_run_pcb=pcb;

    tss.ss0 = KERNEL_DS;                                                    // Redefine TSS parameters to tear down the old current process's stack 
    tss.esp0 = 0x800000 - (0x2000 * (parent_id_temp+1));
    restore_reg(pcb_copy);                                                  // Restore registers so we can return to after the context switch in execute 
                                                        
    return 0;
}


/* 
 *   system_execute (const uint8_t* command)
 *   DESCRIPTION: This function takes in an executable name and sets up its execution. First, it checks to see if the executable is valid. If so, 
 *                we check to see if the maximum supported PCBs (2) has been reached. If not, this executable is assigned a process IF (either 0
 *                or 1). From here, the exact position of the kernel stack for this executable is determined, and it's PCB is initialized. Then, 
 *                this executable is mapped from its physical address to a virtual page with the use of the read_data and load_4MB_syscall_page 
 *                functions. TSS parameters are then edited to reflect the correct future values for SS0 and ESP0, and a contest swtich is performed
 *                to take us to the virtual page and start execution of the executable.    
 *   INPUTS: command - Name of the executable to be executed. 
 *   OUTPUTS: If valid, an executable is run. 
 *   RETURN VALUE: -1 if failed to execute, 0 if successful   
 */
int32_t system_execute (const uint8_t* command){

    uint32_t parent_id_temp = -1;                                           // Initializing helper variables/buffers for future use 
    int i = 0;
    int nbytes = 5;
    uint8_t file_data_buf[5];
    uint8_t elf_buf[3];
    cli();                                                                  // Disable interrupts 
    num_times_ex_called = num_times_ex_called+1;                            // Increment our global execute counter 

    /* Obtain the first argument and second argument in command */
    dentry_t dentry;  
    int idx=0;
    int arg_idx=0;
    uint8_t cmd_buf[10]={'\0'};
    while(command[idx]!=' ' && command[idx]!='\0'){                         // Obtain the first argument in command and store this in cmd_buf 
        cmd_buf[idx]=command[idx];
        idx++;
    }
    while(command[idx]!='\0'){                                              // Obtain the second argument in command and store this in arg_buf
        if(command[idx]!=' '){                                              // arg_buf is static, and is accessible from system_getargs 
            arg_buf[arg_idx]=command[idx];
            arg_idx++;
        }
        idx++;
    }

    /* Check if Executable is Valid */
    if(read_dentry_by_name (cmd_buf, &dentry)==-1){                         // Check if the command exists
        return -1; 
    }
    if(read_data(dentry.inode_num, 0, file_data_buf, nbytes) != nbytes){    // Check if file can be read properly 
        return -1;
    }
    for(i = 0 ; i < 3 ; i++){                                               // Obtain the first 3 bytes of the file and store in elf_buf
        elf_buf[i] = file_data_buf[1+i];
    }
    if(strncmp((const int8_t*)elf_buf, (const int8_t*)"ELF", (uint32_t)3) == 0){  // Check if first 3 bytes = "ELF" (if so -> is executable)
    } else{
        return -1;
    }
    int process = NULL;                                                     // Check if we have reached the maximum supported PCB's 
    if(pcb_array[0]==0){                                                    // If we have NOT, assign the executable a PID (0, 1, 2, or 3)
        pcb_array[0]=1; 
        process=0; 
    }
    else if(pcb_array[1]==0){
        pcb_array[1]=1; 
        process=1; 
    }
    else if(pcb_array[2]==0){                                                    
        pcb_array[2]=1; 
        process=2; 
    }
    else if(pcb_array[3]==0){
        pcb_array[3]=1; 
        process=3; 
    }
    else if(pcb_array[4]==0){
        pcb_array[4]=1; 
        process=4; 
    }
    else if(pcb_array[5]==0){
        pcb_array[5]=1; 
        process=5; 
    }
    else{
        printf("Reached Max Amount of PCBs\n");
        return -1;                                                          // If we have reached the maximum supported PCBs, return -1
    }
   
    /* Initlialize PCB and Kernel Stack for Valid Executable */
    if(!(process==0 || process==1 || process==2)){                          // If this executable is not one of the 3 base shells, save it's parent's PID
        parent_id_temp = pcb->process_num; 
    }
       
    pcb = (pcb_t*)(0x800000 - (0x2000 * num_times_ex_called));              // Initialize PCB pointer (8MB - 8KB*number of times execute called)
    pcb->addr=0x800000 - (0x2000 * num_times_ex_called); 
    pcb->process_num = process;   
    pcb->parent_id = parent_id_temp;                                        // Save the current PID, parent's PID 
    int temp = get_seen_terminal_num(); 
    pcb->terminal = temp;   

    /* update the last run pcb for the terminal that we are currently in */
    terminal_t* curr_terminal = terminal_data(get_seen_terminal_num());
    curr_terminal->last_run_pcb=pcb; 

    pcb->fds[0].flags=1;                                                    // Initlize stdin and stdout in the pcb's FD array 
    pcb->fds[1].flags=1;
    pcb->fds[0].operation_ptr = fun_ptr_arr_terminal;
    pcb->fds[1].operation_ptr = fun_ptr_arr_terminal;
    for(i=2; i<8; i++){         
        pcb->fds[i].flags=0;                                                // Initialize the rest of the flags in the FD array to 0 (unused)
    }
    asm("movl %%ebp, %0;" : "=r" (pcb->ebp):);                              // Save current EBP 

    /* Map Executable to Virtual Memory Page */ 
    load_4MB_syscall_page(process);                                         // Call load_4MB_syscall_page to map physical address of executable to virtual page address
    inode_t* inode = get_inode_info(dentry.inode_num);                      // Aquire the inode info of the executable (number, length of file, etc.)
    uint32_t length = inode->length_bytes;
    pcb->length = length;
    pcb->inode_num = dentry.inode_num;
    uint8_t* page_addr = (uint8_t*)(0x08048000);                            // Save the virtual memory page address in page_addr (the program image is mapped to addr: 0x08048000)
    uint8_t filebuf[40];                                                    // Create filebuf to store the EIP later 
    read_data (dentry.inode_num, 0, filebuf, 40);                           // Store the executable into the virtual memory page/into filebuf
    read_data (dentry.inode_num, 0, page_addr, inode->length_bytes);

    /* Perform Context Switch to Start Execution of Executable */
    uint32_t eflags=0;
    uint32_t eip= (filebuf[27]<<24)+(filebuf[26]<<16)+(filebuf[25]<<8)+filebuf[24];     // Aquire bytes 24-27 from the executable to obtain new EIP
    tss.ss0 = KERNEL_DS;
    tss.esp0 = 0x800000 - (0x2000 * num_times_ex_called);                               // Set esp0 to the base of the NEXT kernel stack to be used the next time execute is called 
    sti();
    context_switch (pcb, eip, USER_CS, eflags, 0x8400000, USER_DS);                     // Perform context switch 

    if(pcb_copy == NULL){                                                   
        return -1;
    }
    return pcb_copy->halt_status;
}


/* 
 *   system_read (int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: This function takes in an fd array index, a buffer (which will be filled bu system_read) and the number of bytes to be read 
 *                (nbytes). It indentifies whether we want to perform a terminal, file, directory or RTC read. Once identified, a function pointer
 *                is followed to the correct read function to used. Buf is then populated with what is read, and the number of bytes actually read 
 *                is returned. 
 *   INPUTS: fd     - Index into the FD array of the current process. 
 *           buf    - Array populated with what is read. 
 *           nbytes - Desired amount of bytes to read.
 *   OUTPUTS: If able, it populates buf with what is read and returns the number of bytes read.  
 *   RETURN VALUE: -1 if failed to read, nbytes if successful   
 */
int32_t system_read (int32_t fd, void* buf, int32_t nbytes){
   
    if(fd>7 || fd<0){                                                       // Check to see if the FD index is valid
        return -1;
    }
    int retval;
    if(fd==0){                                                              // If we want to read from the terminal, jump to terminal_read via function pointer
        pcb->fds[fd].operation_ptr = fun_ptr_arr_terminal;
        retval = pcb->fds[fd].operation_ptr[0](fd, buf, nbytes);
    } else if(fd == 1){                                                     // If we want to use stdout (fd=1), return failure 
        return -1;
    } else if(pcb->fds[fd].flags != 0){                                     // If reading a file, RTC, or directory follow a function pointer table to the correct function
        retval =pcb->fds[fd].operation_ptr[0](fd, buf, nbytes);
    } else{                                                 
        return -1;
    }
    return retval;
}


/* 
 *   system_write (int32_t fd, const void* buf, int32_t nbytes)
 *   DESCRIPTION: This function takes in an fd array index, a buffer (which holds what we want to write) and the number of bytes to be written 
 *                (nbytes). It indentifies whether we want to perform a terminal, file, directory or RTC write. Once identified, a function pointer
 *                is followed to the correct write function to used. Buf is then populated with what is read, and the number of bytes actually read 
 *                is returned. 
 *   INPUTS: fd     - Index into the FD array of the current process. 
 *           buf    - Array populated with what is to be written. 
 *           nbytes - Desired amount of bytes to write.
 *   OUTPUTS: If able, it writes to the terminal, a file, RTC or a directory and returns the number of bytes read.  
 *   RETURN VALUE: -1 if failed to write, nbytes if successful   
 */
int32_t system_write (int32_t fd, const void* buf, int32_t nbytes){
    
    if(fd>7 || fd<1){                                                       // Check to see if the FD index is valid
        return -1;
    }
    int retval;
    if(fd==1){                                                              // If we want to write to the terminal, jump to terminal_write via function pointer
        pcb->fds[fd].operation_ptr = fun_ptr_arr_terminal;
        retval = pcb->fds[fd].operation_ptr[1](fd, buf, nbytes);
    }
    else if (pcb->fds[fd].flags != 0){                                      // If writing to a file, RTC, or directory follow a function pointer table to the correct function
        retval =pcb->fds[fd].operation_ptr[1](fd, buf, nbytes);
    } 
    else{
        return -1;
    }
    return retval;
}


/* 
 *   system_open (const uint8_t* filename)
 *   DESCRIPTION: This function takes in a filename and creates an FD array entry for it within the PCB of the current process.  
 *   INPUTS: filename - The name of the file to make the FD entry for. 
 *   OUTPUTS: If able, it opens and file by placing it in the FD array.  
 *   RETURN VALUE: -1 if failed to write, the fd index if successful   
 */
int32_t system_open (const uint8_t* filename){

    if(*filename == '\0'){                                                  // Check to make sure that the filename is not NULL 
        return -1;
    }
    int fd = -1;  
    dentry_t dentry;                                                        // Call read_dentry_by_name to store the file type in dentry
    (void) read_dentry_by_name (filename, &dentry);
    if(dentry.file_type==0){                                                // We have an RTC file - call rtc_open 
        fd = rtc_open(filename);
        pcb->fds[fd].operation_ptr = fun_ptr_arr_rtc;
    }
    else if(dentry.file_type==1){                                           // We have a directory file - call dir_open 
        fd = dir_open(filename);
        pcb->fds[fd].operation_ptr = fun_ptr_arr_dir;
    }
    else if(dentry.file_type==2){                                           // We have a normal file - call file_open 
        fd = file_open(filename);
        pcb->fds[fd].operation_ptr = fun_ptr_arr_file;
    }
    return fd; 
}


/* 
 *   system_close (int32_t fd)
 *   DESCRIPTION: This function closes a file at fd (index into FD array) by setting the flag low.  
 *   INPUTS: fd - The index into the current process' FD array that we want to close. 
 *   OUTPUTS: If able, it closes and file by setting its' flag low  
 *   RETURN VALUE: -1 if failed to write, 0 if successful   
 */
int32_t system_close (int32_t fd){

    if(fd>7 || fd<2){                                                       // Check to see if the fd index is valid 
        return -1;
    }
    if(pcb->fds[fd].flags != 0){                                            // Use a function pointer to call the correct close() function (file, directory, RTC)
        return pcb->fds[fd].operation_ptr[3](fd);
    }
    return -1;
}


/* 
 *   system_getargs (uint8_t* buf, int32_t nbytes)
 *   DESCRIPTION: This function obtains the second argument in the command input to system_execute(...).  
 *   INPUTS: buf    - buffer to store the second argument of command 
 *           nbytes - the size of the buffer to store the second argument 
 *   OUTPUTS: This function stores the second argument of command (from system_execute) into buf, such that it can be used later.   
 *   RETURN VALUE: -1 if failed to read the second argument, 0 if successful   
 */
int32_t system_getargs (uint8_t* buf, int32_t nbytes){

    int i;                                                                  // MAGIC NUMBER: The keyboard and arg_buf can only store 128 chars 
    int null_counter=0;
    for(i=0;i<33;i++){                                                     // Store the contents of arg_buf into buf (arg_buf is filled in system_execute)
        buf[i]=arg_buf[i];
        if(arg_buf[i]=='\0'){                                               // Count the amount of null chars in buf 
            null_counter++;
        }
    }
    for(i=0;i<33;i++){                                                     // Clear arg_buf such that it can be used later 
        arg_buf[i]='\0';
    }
    if((null_counter==33)||(null_counter==0)){                             // If buf is filled with all nulls (is empty) or does not end with a null terminator, return -1
        return -1;
    }
    return 0;                                                               // Return 0 upon success 
}


/* 
 *   system_vidmap (uint8_t** screen_start)
 *   DESCRIPTION: This function maps a user accessible virtual page (4kB) to the physical VIDEO_MEMORY page.  
 *   INPUTS: screen_start - Pointer to a pointer which will eventually store the virtual address of the user accessible page 
 *   OUTPUTS: This function creates a user accessible virtual page that is mapped to the physical page that stores VIDEO_MEMORY   
 *   RETURN VALUE: -1 if the address of screen_start is nto within the program image, 0 otherwise.    
 */
int32_t system_vidmap (uint8_t** screen_start){

    if((uint32_t)screen_start<(0x8000000)||(uint32_t)screen_start>(0x8000000+ 0x400000)){      // Check screen_start is within valid range [128 MB, 132 MB]
        return -1;                                                         // 128 MB = 0x80000000, 132 MB = 0x8000000+ 0x400000
    }
    *screen_start=(uint8_t*)0x08822000;                                    // Map the virtual user level page to 0x08822000 (because index 34 in page_directory corresponds to this value)
    vidmap_page_table_init();                                              // Create a vidmap page table to hold the new virtual 4kB page 
    vidmap_directory_entry_init(34);                                       // Initialize the page directory entry for this new page table 
    vidmap_init(34);                                                       // Create the new page inside of the new page table and map this to index 34 (because index 34 in page_directory corresponds to this value).
    return 0;
}


/* 
 *   system_set_handler (int32_t signum, void* handler_address)
 *   DESCRIPTION: UNUSED BECAUSE SIGNALS ARE NOT SUPPORTED   
 *   INPUTS: signum          - N/A
 *           handler_address - N/A
 *   OUTPUTS: N/A 
 *   RETURN VALUE: Will always return -1   
 */
int32_t system_set_handler (int32_t signum, void* handler_address){
    return -1;
}


/* 
 *   system_sigreturn (void)
 *   DESCRIPTION: UNUSED BECAUSE SIGNALS ARE NOT SUPPORTED   
 *   INPUTS:  N/A
 *   OUTPUTS: N/A 
 *   RETURN VALUE: Will always return -1   
 */
int32_t system_sigreturn (void){
    return -1;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// CODE USED FOR DEBUGGING/////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//printing the file content for debug
    // for(i=0; i<nbytes;i++){
    //     if(file_data_buf[i]!='\0'){
    //         printf("%c",file_data_buf[i]);
    //         counter++;
    //     }

    //     printf("%c",file_data_buf[i]);
    //     counter++;

    //     if(counter%80==0 && i!=0){
    //         printf("\n");
    //     }
    // }

