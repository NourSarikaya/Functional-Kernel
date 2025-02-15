#include "filesystem.h"
#include "syscall.h"
#include "rtc.h"
#include "terminal.h"

/* File System Driver Global Variables */
static int counter = 0; 
static boot_block_t* block_ptr; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////// FILE FUNCTIONS //////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* 
 * get_inode_info
 *   DESCRIPTION: Get the corresponding element we want in the inode array
 *   INPUTS: inode_num: the index for the inode we want to access
 *   OUTPUTS: none
 *   RETURN VALUE: a pointer to the element we want in the inode array
 *   SIDE EFFECTS: none
 */

inode_t* get_inode_info(int32_t inode_num){
    return (inode_t*)block_ptr + inode_num + 1; // +1 account for the boot block
}

/* 
 * filesystem_init
 *   DESCRIPTION: Initialize the global boot block pointer. 
 *   INPUTS: filesystem_addr: the base address for the boot block
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: n/a
 */
void filesystem_init(unsigned int filesystem_addr){
    block_ptr = (boot_block_t*)filesystem_addr; 
}

/* 
 * file_read
 *   DESCRIPTION: read the file and store the result inside buf
 *   INPUTS: fd: the file that we are trying to read
 *           buf: the buffer to store the file content to
 *           nbytes: number of bytes to copy to buf
 *   OUTPUTS: none
 *   RETURN VALUE: return the length of the copy on successful read
 *                 return 0 on fail
 *   SIDE EFFECTS: reads a file by storing contents in buf with nbytes length
 */
int32_t file_read (int32_t fd, void* buf, int32_t nbytes){
    int32_t length = read_data (pcb_ptr()->fds[fd].inode_num, pcb_ptr()->fds[fd].file_pos, (uint8_t*) buf, nbytes);
    pcb_ptr()->fds[fd].file_pos += length;
    if(length == 0)
        pcb_ptr()->fds[fd].file_pos = 0;
    return length; 
    //return 0; 
}

/* 
 * file_write
 *   DESCRIPTION: NOT IMPLEMENTED FOR NOW
 *   INPUTS: fd: N/A
 *           buf: N/A
 *           nbytes: N/A
 *   OUTPUTS: none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: N/A
 */
int32_t file_write (int32_t fd, const void* buf, int32_t nbytes){
    return -1; 
}

/* 
 * file_open
 *   DESCRIPTION: Open the file based on filename, store it's data in the FD array of the current process. 
 *   INPUTS: filename - the file we are trying to open
 *   OUTPUTS: none
 *   RETURN VALUE: success: return the fd we assigned to this file 
 *                 fail(cannot find available space in fd): return 0
 *                 fail(cannot find file with filename): return -1
 *   SIDE EFFECTS: store the file's info into the fd array
 */
int32_t file_open (const uint8_t* filename){
    dentry_t dentry;
    int retval = read_dentry_by_name (filename, &dentry);                       // Call read_dentry_by_name to see if file exists 
    if (retval != 0 || dentry.file_type!=2) {                                   // If not exist or is not a normal file, we fail 
        return -1;
    }
    int i; 
    for(i=2; i<8; i++){                                                         // Look through thd FD array to find an open spot to store the file details 
        if(pcb_ptr()->fds[i].flags==0){
            pcb_ptr()->fds[i].inode_num = dentry.inode_num;                     // Initialize the paramaters within this index
            pcb_ptr()->fds[i].file_pos = 0; 
            pcb_ptr()->fds[i].flags = 1;
            return i;                                                           // Return the FD array index 
        }
    }
    return -1;
}

/* 
 * file_close
 *   DESCRIPTION: Close the file corresponding to fd, which exists in the FD array of the current process. 
 *   INPUTS: fd - the index of the file in the fd array we want to close
 *   OUTPUTS: none
 *   RETURN VALUE: success: return 0
 *   SIDE EFFECTS: clear the flag of the fd entry in the fd array
 */
int32_t file_close (int32_t fd){
    if(pcb_ptr()->fds[fd].flags !=0 ){              // Access the flags value located within the FD array of our current process 
        pcb_ptr()->fds[fd].flags = 0;               // Set the flag to 0
        return 0; 
    }
    return -1; 
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////// DIRECTORY FUNCTIONS ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* 
 * dir_read
 *   DESCRIPTION: store the name of a file in the current directory inside buffer
 *   INPUTS: fd: the file that we are trying to read
 *           buf: the buffer to store the file's name to
 *           nbytes: number of bytes to copy to buf
 *   OUTPUTS: none
 *   RETURN VALUE: return 0 on success
 *                 return -1 on fail
 *   SIDE EFFECTS: store the name of the file inside buf and 
 *                 increment counter to the next file in the directory
 */
int32_t dir_read (int32_t fd, void* buf, int32_t nbytes){
    int i = 0;
    int file_name_size = 0;
    int file_type = -1;
    int file_size = -1;
    int inode = -1;
    dentry_t entry; 
    inode_t* index_node;

    // return fail if we already traverse all files in this directory
    if(counter>=(block_ptr->stats).num_dirs){
        counter = 0;
        return 0; 
    }

    for(i = 0 ; i < 32 ; i++){
        ((uint8_t*) buf)[i] = ' ';
    }

    // get the directory entry using counter as index
    read_dentry_by_index (counter, &entry);
    file_type = entry.file_type;
    // dont copy the file name if it is rtc
    if(file_type == 0 || file_type == 1){
        file_size = 0;
    } else {
        // the file is a normal file, find the corresponding inode num
        inode = entry.inode_num;

        // find the corresponding inode
        index_node = (inode_t* )block_ptr + inode + 1;

        // get the file size
        file_size = index_node->length_bytes;
    }

    // get the length of the file name for formating
    for(i = 0 ; i < 32 ; i++){
        if(entry.file_name[i] == '\0'){
            break;
        }
        file_name_size++;
    }

    // print the file_name_size for debug
    // copy file name into buffer (32 char in entry)
    for(i = 0 ; i < file_name_size ; i++){
       ((uint8_t*) buf)[i] = entry.file_name[i];
    }

    counter++; 

    return file_name_size; 
}



/* 
 * dir_write
 *   DESCRIPTION: NOT IMPLEMENTED FOR NOW
 *   INPUTS: fd: N/A
 *           buf: N/A
 *           nbytes: N/A
 *   OUTPUTS: none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: N/A
 */
int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes){
    return -1; 
}



/* 
 * dir_open
 *   DESCRIPTION: open the directory based on filename
 *   INPUTS: filename: the directory we are trying to open
 *   OUTPUTS: none
 *   RETURN VALUE: success: return the fd we assigned to this file 
 *                 fail(cannot find available space in fd): return 0
 *                 fail(cannot find directory with filename): return -1
 *   SIDE EFFECTS: store the directory's info into the fd array
 */
int32_t dir_open (const uint8_t* filename){
    dentry_t dentry;
    int i, retval; 

    retval = read_dentry_by_name (filename, &dentry);
    // file type 1 = directory
    if (retval != 0 || dentry.file_type!=1) {
        return -1;
    }

    // 2 as stdin and stdout is the first 2, 8 as the array length is 8
    for(i=2; i<8; i++){     
        if(pcb_ptr()->fds[i].flags==0){
            // ignore index node number as we are dealing with directory
            pcb_ptr()->fds[i].inode_num=0; 
            pcb_ptr()->fds[i].file_pos = 0; 
            pcb_ptr()->fds[i].flags = 1;  //Mark file as in use
            return i; 
        }
    }
    // return fail if maximum number of opened file is reached
    return -1;
}

/* 
 * dir_close
 *   DESCRIPTION: close the directory based on filename
 *   INPUTS: fd: the index of directory in the fd array we are trying to close
 *   OUTPUTS: none
 *   RETURN VALUE: success: 0
 *   SIDE EFFECTS: clear the flag inside the corresponding fd element in the array
 */
int32_t dir_close (int32_t fd){
    // clear flag
    if(pcb_ptr()->fds[fd].flags !=0 ){
        pcb_ptr()->fds[fd].flags = 0;
        return 0; 
    }

    // else failed
    return -1; 
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////// HELPER FUNCTIONS ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* 
The three routines provided by the file system module return -1 on failure, indicating a non-existent file or invalid
index in the case of the first two calls, or an invalid inode number in the case of the last routine. Note that the directory
entries are indexed starting with 0. Also note that the read data call can only check that the given inode is within the
valid range. It does not check that the inode actually corresponds to a file (not all inodes are used). However, if a bad
data block number is found within the file bounds of the given inode, the function should also return -1.

When successful, the first two calls fill in the dentry t block passed as their second argument with the file name, file
type, and inode number for the file, then return 0. The last routine works much like the read system call, reading up to
length bytes starting from position offset in the file with inode number inode and returning the number of bytes
read and placed in the buffer. A return value of 0 thus indicates that the end of the file has been reached.
*/

/* 
 * read_dentry_by_name
 *   DESCRIPTION: find the corresponding directory entry by name of the file
 *   INPUTS: fname: the name of the file we are looking for
 *   OUTPUTS: none
 *   RETURN VALUE: success: 0
 *                 fail: -1
 *   SIDE EFFECTS: store the pointer of the corresponding directory entry by name of the file
 *                 inside dentry
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    //traverse every dentry in the boot block 
    int i;
    for (i = 0; i < 63; i++) { //There are 63 dentries in total

        //search for the dentry with the matching file name
        if (strncmp((const int8_t*)fname, (const int8_t*)(block_ptr->dir[i]).file_name, 32)==0){
            //copy contents of current dentry into parameter "dentry" 
            *dentry = block_ptr->dir[i];
            
            return 0;
        }

    }
            
    return -1; 
}


/* 
 * read_dentry_by_index
 *   DESCRIPTION: find the corresponding directory entry by index of the file
 *   INPUTS: index: the index of the file we are looking for
 *   OUTPUTS: none
 *   RETURN VALUE: success: 0
 *                 fail: -1
 *   SIDE EFFECTS: store the pointer of the corresponding directory entry by index of the file
 *                 inside dentry
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){

    //check to see if index is in range [0,62]
    if(index<0 || index>62){ 
        return -1;
    }
    *dentry = block_ptr->dir[index];
      
    return 0;

}


/* 
 * read_data
 *   DESCRIPTION: Read the file with inode starting from offset and store the result inside buf
 *                with copied length == length
 *   INPUTS: inode: the index of inode of the file that we are trying to read
 *           offset: the offset the start reading the file from
 *           buf: the buffer to store the file content to
 *           length: number of bytes to copy to buf
 *   OUTPUTS: none
 *   RETURN VALUE: success: return the length we copied
 *                 fail(offest is invalid or the file ended before copy can finish): return 0
 *                 fail(cannot find inode with "inode" index): return -1
 *   SIDE EFFECTS: reads a file by storing contents in buf with length character
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    // need base addr for our file blocks !!!!!!!!
    //dentry_t* dentry;
    inode_t* index_node;
    data_block_t* data_block_base_addr;

    int i;
    int file_data_length;
    int data_block_cnt, data_block_offset, cur_data_block_index;

    // check if the inode index is valid
    if (inode<0||inode>(const uint8_t)(block_ptr->stats).num_inodes -1){ //inodes go up to N-1
        return -1;
    }
    
    // get the corresponding inode ( plus one as inode index 0 starts at 1 pass boot block)
    index_node = (inode_t* )block_ptr + inode + 1;

    // get length of file
    file_data_length = index_node->length_bytes;

    // sets the length to read to the file data's length if it's larger
    if(length>file_data_length){
        length=file_data_length;
    }

    // check if the offset is valid
    if(offset < 0) return 0;

    // check if the file end before we can copy "length" member into buf
    if((offset) >= file_data_length) return 0;


    // set the base addr of data block
    data_block_base_addr = (data_block_t*)block_ptr + (block_ptr->stats).num_inodes +1; //+1 account for the bootblock

    // get the correct data block and data block offset
    data_block_cnt = offset / DATA_BLK_SIZE;
    cur_data_block_index = index_node->data_block_nums[data_block_cnt];
    data_block_offset = offset % DATA_BLK_SIZE;

    // copy data into buffer
    for(i = 0 ; i < length ; i++){

        // jump to the next data block if the end of block is reached
        if(data_block_offset >= DATA_BLK_SIZE){
            data_block_offset = 0;
            data_block_cnt++;
            cur_data_block_index = index_node->data_block_nums[data_block_cnt];
        }

        // write the data_block_offset member of the cur_data_block_index block into the buffer
        buf[i] = data_block_base_addr[cur_data_block_index].data[data_block_offset];

        // increment data_block_offset
        data_block_offset++;
    }

    // return the length of file 
    return length;
}

/*function pointers to all 4 main functions in the order of read, write, open, close*/

/*file's function pointers*/
int32_t (*fun_ptr_arr_file[4])() = {file_read, file_write, file_open, file_close}; 

/*directory's function pointers*/
int32_t (*fun_ptr_arr_dir[4])() = {dir_read, dir_write, dir_open, dir_close}; 

/*rtc's function pointers*/
int32_t (*fun_ptr_arr_rtc[4])() = {rtc_read, rtc_write, rtc_open, rtc_close};

/*terminal's function pointers*/
int32_t (*fun_ptr_arr_terminal[4])() = {terminal_read, terminal_write, terminal_open, terminal_close};

