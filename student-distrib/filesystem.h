#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "types.h"
#include "lib.h"
#define DATA_BLK_SIZE 4096 
#define MAX_NUM_DIR 63
#define MAX_NUM_DATABLK 1023

/*declare the function pointers*/
/*file's function pointers*/
extern int32_t (*fun_ptr_arr_file[4])();
/*directory's function pointers*/
extern int32_t (*fun_ptr_arr_dir[4])();
/*rtc's function pointers*/
extern int32_t (*fun_ptr_arr_rtc[4])();
/*terminal's function pointers*/
extern int32_t (*fun_ptr_arr_terminal[4])();


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////// FILE SYSTEM STRUCTS ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct file_system_stats_t {
    /* File System Specs */
    uint32_t num_dirs;
    uint32_t num_inodes;
    uint32_t num_data_blocks; 
    
    /* Reserved 52B */
    uint32_t reserved_0; 
    uint32_t reserved_1; 
    uint32_t reserved_2; 
    uint32_t reserved_3; 
    uint32_t reserved_4; 
    uint32_t reserved_5; 
    uint32_t reserved_6; 
    uint32_t reserved_7; 
    uint32_t reserved_8; 
    uint32_t reserved_9; 
    uint32_t reserved_10; 
    uint32_t reserved_11; 
    uint32_t reserved_12;  
} file_system_stats_t;


typedef struct dentry_t {
    /* File Specs */
    uint8_t file_name[32]; 
    uint32_t file_type; 
    uint32_t inode_num;  
    
    /* Reserved 24B */
    uint32_t reserved_0; 
    uint32_t reserved_1; 
    uint32_t reserved_2; 
    uint32_t reserved_3; 
    uint32_t reserved_4; 
    uint32_t reserved_5;  
} dentry_t;


typedef struct inode_t {
    /* Inode Contents */
    uint32_t length_bytes; 
    uint32_t data_block_nums[MAX_NUM_DATABLK];
} inode_t;

typedef struct data_block_t {
    /* data block Contents */
    uint8_t data[DATA_BLK_SIZE];
} data_block_t;

typedef struct boot_block_t {
    /* Contents of Boot Block */
    file_system_stats_t stats; 
    dentry_t dir[MAX_NUM_DIR]; 
} boot_block_t;

typedef struct file_desc_t {
    /* File Descriptor Specs */
    int32_t (**operation_ptr)(); 
    uint32_t inode_num;
    uint32_t file_pos; 
    uint32_t flags;
  
} file_desc_t;

/* Get the corresponding element we want in the fd array */
file_desc_t* get_fd_info(int index); 

/* Get the corresponding element we want in the inode array */
inode_t* get_inode_info(int32_t inode_num);

/* Initialize the fd array */
void filesystem_init(unsigned int filesystem_addr);

/* Read the file and store the result inside buf */
int32_t file_read (int32_t fd, void* buf, int32_t nbytes);

/* NOT IMPLEMENTED FOR NOW */
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);

/* Open the file based on filename */
int32_t file_open (const uint8_t* filename);

/* Close the file based on file descriptor */
int32_t file_close (int32_t fd);

/* Store the name of a file in the current directory inside buffer */
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes);

/* Does nothing */
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes);

/* Open the directory with filename */
int32_t dir_open (const uint8_t* filename);

/* Close the directory with filename */
int32_t dir_close (int32_t fd);

/* Find the corresponding directory entry by name of the file */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

/* Find the corresponding directory entry by index of the file */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

/* Read the file with inode starting from offset and store the result inside buf with copied length == length */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif
