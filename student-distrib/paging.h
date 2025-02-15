#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define ENTRIES_NUM   1024
#define VIDEO_MEM_ADDR   0xB8

/* initializing a 32 bit page directory entry struct */
typedef struct __attribute__ ((packed)) page_directory_entry {
        uint32_t present          : 1;
        uint32_t read_write       : 1;
        uint32_t user_supervisor  : 1;
        uint32_t write_through    : 1;
        uint32_t cache_disabled   : 1;
        uint32_t accessed         : 1;
        uint32_t reserved         : 1;
        uint32_t page_size        : 1;
        uint32_t global_page      : 1;
        uint32_t avail            : 3;
        uint32_t page_table_base_addr          : 20;
} page_directory_entry;

/* initializing a 32 bit page table entry struct */
typedef struct __attribute__ ((packed)) page_table_entry {
        uint32_t present          : 1;
        uint32_t read_write       : 1;
        uint32_t user_supervisor  : 1;
        uint32_t write_through    : 1;
        uint32_t cache_disabled   : 1;
        uint32_t accessed         : 1;
        uint32_t dirty            : 1;
        uint32_t page_table_attr_index         : 1;
        uint32_t global_page      : 1;
        uint32_t avail            : 3;
        uint32_t page_base_addr          : 20;
} page_table_entry;

/* Initlialize the Page Directory (each point to an area in the Page Table) */
page_directory_entry page_directory[1024] __attribute__((aligned(4096))) ;

/* Initialize the Page Table (4KB Pages) */
page_table_entry first_page_table[1024] __attribute__((aligned(4096))) ;
page_table_entry vidmap_page_table[1024] __attribute__((aligned(4096))) ;


/* Loads CR3 with the Page Directory Base Address */
extern void loadPageDirectory(page_directory_entry*);

/* map the 4mb syscall page to a particular physical address based on the process number given */
extern void load_4MB_syscall_page(int process);

/* Enables Paging by Setting Attributes in CR0 and CR4 */
extern void enablePaging();

/* flushing tlb by resetting cr3 */
extern void tlb_flush();

/* Initializes Paging */
extern void page_init();

/* Initializes page directory */
void page_directory_init();

/* Initializes page directory */
void vidmap_init(uint32_t screen_start);

/* Initializes the vidmap page table for our vidmap sycall */
void vidmap_page_table_init(); 

/* Deallocates the page for vidmap */
void deallocate_page(int index);

/* Remaps program image to specified terminal's physical address*/
void copy_terminal_data(int terminal_num);

/* Creates page directory entry for the page table which holds the new vidmap page */
void vidmap_directory_entry_init(uint32_t screen_start);

/* Initializes first page table */
void first_page_table_init();

/* Initializes video memory page table entry */
void video_mem_page_table_entry_init(int terminal_num);

/* Initializes video memory page directory entry */
void video_mem_page_directory_entry_init();

/* Initializes kernel page directory entry */
void kernel_page_directory_entry_init();

/* Returns the address of the page for the terminal associated with num */
uint8_t* terminal_addr_ptr(int num);

#endif /* PAGING_H */


