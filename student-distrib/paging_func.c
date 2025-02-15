#include "paging.h"
#include "lib.h"

static uint8_t* terminal1_addr = (uint8_t*)(0x0B8000+0x01000);    // The virtual address of terminal 1's page
static uint8_t* terminal2_addr = (uint8_t*)(0x0B8000+2*0x01000);  // The virtual address of terminal 2's page
static uint8_t* terminal3_addr = (uint8_t*)(0x0B8000+3*0x01000);  // The virtual address of terminal 3's page


/* 
 * terminal_addr_ptr
 *   DESCRIPTION: Returns the Virtual Video Memory address of Terminal pages
 *   INPUTS: num- which terminal's virtual page address we want to return
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to virtual address of the specified terminal's page
 */
uint8_t* terminal_addr_ptr(int num){
    if(num==1){
        return terminal1_addr;                  
    }
    else if(num==2){
        return terminal2_addr; 
    }
    else {
        return terminal3_addr; 
    }
}


/* 
 * de_allocate_page
 *   DESCRIPTION: Sets the present bit of vidmap's page directory and page table to 0
 *   INPUTS: index - index into the page directory and page table
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes the present bit of page table and page directory
 */
void deallocate_page(int index){
    page_directory[index].present = 0;
    vidmap_page_table[index].present = 0;
}

/* 
 * copy_terminal_data
 *   DESCRIPTION: maps the Virtual Video Memory address to one of the Physical Terminal Addresses
 *   INPUTS: terminal_num- which terminal's background buffere we want the video memory to point to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes the video memory mapping
 */
void copy_terminal_data(int terminal_num){
    first_page_table[VIDEO_MEM_ADDR].page_base_addr = VIDEO_MEM_ADDR+terminal_num*0x01; // Page table register index= 0xB8 + 0x1, terminal 1: 0xB9, terminal 2: 0xBA, terminal 3: 0xBB
    tlb_flush();                                                               
}



/* 
 * page_directory_init
 *   DESCRIPTION: Initialize page directory entries (PDE)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the PDE by setting every bit to 0 except the size bit
 *                 Initialize all PDE to indicate a 4 MB page
 */
void page_directory_init(){
    unsigned int i;

    // Loop through the entire Page Directory (from index 0 to 1023)
    for(i = 0; i < ENTRIES_NUM; i++)
    {
        // initialize all entries and the Present bit to 0
        page_directory[i].present = 0;
        page_directory[i].read_write = 0;
        page_directory[i].user_supervisor = 0;
        page_directory[i].write_through = 0;
        page_directory[i].cache_disabled = 0;
        page_directory[i].accessed = 0;
        page_directory[i].reserved = 0;
        // initialize all PDE as 4MB page PDE first
        page_directory[i].page_size = 1;
        page_directory[i].global_page = 0;
        page_directory[i].avail = 0;
        page_directory[i].page_table_base_addr = 0;
    }
    tlb_flush();
}



/* 
 * first_page_table_init
 *   DESCRIPTION: Initialize page table entries (PTE)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the PTE by setting every bit to 0 
 */
void first_page_table_init(){
    unsigned int i;

    // Loop through the entire Page Table (from index 0 to 1023) 
    for(i = 0; i < ENTRIES_NUM; i++)
    {
        //and initialize all entries and the Present bit to 0
        first_page_table[i].present = 0;
        first_page_table[i].read_write = 0;
        first_page_table[i].user_supervisor = 0;
        first_page_table[i].write_through = 0;
        first_page_table[i].cache_disabled = 0;
        first_page_table[i].accessed = 0;
        first_page_table[i].dirty = 0;
        first_page_table[i].page_table_attr_index = 0;
        first_page_table[i].global_page = 0;
        first_page_table[i].avail = 0;
        first_page_table[i].page_base_addr = 0;     
    }
    tlb_flush();
}



/* 
 * video_mem_page_table_entry_init
 *   DESCRIPTION: Initialize page table entry for video memory
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the table entry corresponding to video memory
 *                 See the comment below for more detail
 */
void video_mem_page_table_entry_init(int terminal_num){
    /* 
    * Set the supervisor level, Read/Write, and Present Bit of the Video Memory PTE
    *
    * NOTE: ORing with 3 sets the supervisor level, Read/Write, and Present Bit 
    *   Supervisor Level = 0 = page may be accessed by all 
    *   Read/Write       = 1 = can read and write to page 
    *   Present          = 1 = page is mapped to this descriptor
    *
    *   Refer to: https://wiki.osdev.org/Paging#Page_Directory
    *
    * NOTE: 0xB8000 = Video Memory Address, 0xB8 = Index in Page Table for Video Memory Descriptor
    *       0xB9000 = Terminal 1 Background Buffer, 0xB9= Index in Page Table
    *       0xBA000 = Terminal 2 Background Buffer, 0xB9= Index in Page Table
    *       0xBB000 = Terminal 3 Background Buffer, 0xB9= Index in Page Table
    */

    /*set the first_page_table index at "Index in Page Table for Video Memory Descriptor (0xB8)" to the 
    * sum of the "set supervisor level, Read/Write, and Present Bit of the Video Memory PTE (0x00000003)" 
    * and the "Video Memory Address (0xB8000)"*/
    first_page_table[VIDEO_MEM_ADDR+terminal_num*0x01].present = 1;
    first_page_table[VIDEO_MEM_ADDR+terminal_num*0x01].read_write = 1;
    first_page_table[VIDEO_MEM_ADDR+terminal_num*0x01].user_supervisor = 0;
    first_page_table[VIDEO_MEM_ADDR+terminal_num*0x01].page_base_addr = VIDEO_MEM_ADDR+terminal_num*0x01; 
    tlb_flush();
}


/* 
 * vidmap_init
 *   DESCRIPTION: Initialize page within vidmap_page_table for vidmap
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the page within the vidmap page table for the vidmap syscall 
 */
void vidmap_init(uint32_t screen_start){
    /*
    * Mark the page as present, allow reading/writing to the page, set to user level, and map to physical address VIDEO_MEM_ADDR. 
    * Flush the TLB since we are adding a page. 
    */
    vidmap_page_table[screen_start].present = 1;                                 
    vidmap_page_table[screen_start].read_write = 1;
    vidmap_page_table[screen_start].user_supervisor = 1;
    vidmap_page_table[screen_start].page_base_addr = VIDEO_MEM_ADDR;
    tlb_flush();                    
}




/* 
 * vidmap_directory_entry_init
 *   DESCRIPTION: Initialize page directory entry for video memory's table entry
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the directory entry corresponding to video memory's table entry
 *                 See the comment below for more detail
 */
void vidmap_directory_entry_init(uint32_t screen_start){
    /* 
    * Index 0 of the Page Directory points to the KB Page Table
    *
    * NOTE: ORing with 3 sets the supervisor level, Read/Write, and Present Bit 
    *   Supervisor Level = 0 = page may be accessed by all 
    *   Read/Write       = 1 = can read and write to page 
    *   Present          = 1 = page is mapped to this descriptor
    *
    *   Refer to: https://wiki.osdev.org/Paging#Page_Directory
    */

    // binary value of 011 is equivalent to decimal 3
    unsigned int vidmap_page_table_ptr = (unsigned int)vidmap_page_table;
    vidmap_page_table_ptr = vidmap_page_table_ptr >> 12; // right shift 12 bit as the lower 12 bits of pde is used
    page_directory[screen_start].present = 1;
    page_directory[screen_start].read_write = 1;
    page_directory[screen_start].user_supervisor = 1;
    // set page size back to 0 as we are using 4kb pages here as specified
    page_directory[screen_start].page_size = 0;
    page_directory[screen_start].page_table_base_addr = vidmap_page_table_ptr;
    tlb_flush();
}



/* 
 * vidmap_page_table_init
 *   DESCRIPTION: Initialize page table entries (PTE)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the PTE by setting every bit to 0 
 */
void vidmap_page_table_init(){
    unsigned int i;

    // Loop through the entire Page Table (from index 0 to 1023) 
    for(i = 0; i < ENTRIES_NUM; i++)
    {
        //and initialize all entries and the Present bit to 0
        vidmap_page_table[i].present = 0;
        vidmap_page_table[i].read_write = 0;
        vidmap_page_table[i].user_supervisor = 0;
        vidmap_page_table[i].write_through = 0;
        vidmap_page_table[i].cache_disabled = 0;
        vidmap_page_table[i].accessed = 0;
        vidmap_page_table[i].dirty = 0;
        vidmap_page_table[i].page_table_attr_index = 0;
        vidmap_page_table[i].global_page = 0;
        vidmap_page_table[i].avail = 0;
        vidmap_page_table[i].page_base_addr = 0;     
    }
    tlb_flush();
}



/* 
 * video_mem_page_directory_entry_init
 *   DESCRIPTION: Initialize page directory entry for video memory's table entry
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the directory entry corresponding to video memory's table entry
 *                 See the comment below for more detail
 */
void video_mem_page_directory_entry_init(){
    /* 
    * Index 0 of the Page Directory points to the KB Page Table
    *
    * NOTE: ORing with 3 sets the supervisor level, Read/Write, and Present Bit 
    *   Supervisor Level = 0 = page may be accessed by all 
    *   Read/Write       = 1 = can read and write to page 
    *   Present          = 1 = page is mapped to this descriptor
    *
    *   Refer to: https://wiki.osdev.org/Paging#Page_Directory
    */

    // binary value of 011 is equivalent to decimal 3
    unsigned int first_page_table_ptr = (unsigned int)first_page_table;
    first_page_table_ptr = first_page_table_ptr >> 12; // right shift 12 bit as the lower 12 bits of pde is used
    page_directory[0].present = 1;
    page_directory[0].read_write = 1;
    page_directory[0].user_supervisor = 0;
    // set page size back to 0 as we are using 4kb pages here as specified
    page_directory[0].page_size = 0;
    page_directory[0].page_table_base_addr = first_page_table_ptr;
    tlb_flush();
}



/* 
 * kernel_page_directory_entry_init
 *   DESCRIPTION: Initialize page directory entry for kernel
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the directory entry corresponding to kernel
 *                 See the comment below for more detail
 */
void kernel_page_directory_entry_init(){
    /* 
    * Index 1 of the Page Directory points to the MB Page Table 
    *
    * NOTE: ORing with 131 (equivalent to binary 1000 0011) sets the Page Size Attribute, Read/Write, and Present Bit 
    *   Page Size        = 1 = PDE maps to a page table where pages are 4 MB in size  
    *   Read/Write       = 1 = can read and write to page 
    *   Present          = 1 = page is mapped to this descriptor
    *
    *   Refer to: https://wiki.osdev.org/Paging#Page_Directory
    */

    page_directory[1].present = 1;
    page_directory[1].read_write = 1;
    // 1 as linear address 4 mb (0x00400000) must be mapped to physical addr 0x00400000
    // Thus, the 12 bit in page base addr of a PDE with 4-Mbyte page need to be the same
    // as the upper 12 bit of physical addr. Therefore, bit 22 needs to be 1.
    // left shift by 10 as the page base addr declared start at bit 12, 22-12 = 10
    page_directory[1].page_table_base_addr = (1 << 10); // 1 as linear address 4 mb
    tlb_flush();
}



/* 
 *  load_4MB_syscall_page
 *   DESCRIPTION: map the virtual address of 128 MB to physical address starting from 8 MB
 *   INPUTS: process
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: map the virtual address 128MB to the correct physical address
 */
void load_4MB_syscall_page(int process){
    // 32 as 128(wanted virtual address) / 4(4 mb per page) = 32
    page_directory[32].present = 1;
    page_directory[32].read_write = 1;           // read and write 
    page_directory[32].user_supervisor = 1;      // user 
    page_directory[32].reserved = 1;
    // 1 as linear address must be mapped to physical addr 0x00800000 + 0x00400000 * process
    // Thus, the 12 bit in page base addr of a PDE with 4-Mbyte page need to be the same
    // left shift by 10 as the page base addr declared start at bit 12, 22-12 = 10
    // 2 as 2 << 10 is 0x00800000
    page_directory[32].page_table_base_addr =((2+process) << (10));
    tlb_flush();
} 



// see https://courses.grainger.illinois.edu/ECE391/sp2024/secure/references/IA32-ref-manual-vol-3.pdf page 87, 88, 90, 91
/*
*   page_init()
*
*   DESCRIPTION/FUNCTIONALITY: 
*   This function initializes paging for our operating system. It specifically initializes the Page Directory, 
*   setting its' 0th index to the Descriptor for the 4KB Page Table, and the 1st index to the 4MB Page Table.
*   This function also calls 2 helper functions to load the base address of the Page Directory into CR3, as well
*   as set attributes in CR0 and CR4.   
*
*   INPUTS:   
*   N/a
*
*   OUTPUTS: 
*   N/a
*/
void page_init(){

    // initialize page directory
    page_directory_init();

    // initialize the first page table
    first_page_table_init();

    // initialize page directory entry 0 for video memory
    video_mem_page_directory_entry_init();

    // initialize page table entry 0xB8 for video memory
    video_mem_page_table_entry_init(0);
    // initialize page table entry 0xB9 for video memory
    video_mem_page_table_entry_init(1);
    // initialize page table entry 0xBA for video memory
    video_mem_page_table_entry_init(2);
    // initialize page table entry 0xBB for video memory
    video_mem_page_table_entry_init(3);
    
    // initialize page directory entry 1 for kernel
    kernel_page_directory_entry_init();
    
    // Load the Page Directory Base Address into CR3 
    loadPageDirectory(page_directory);

    // Enable Paging for the OS 
    enablePaging();
}
