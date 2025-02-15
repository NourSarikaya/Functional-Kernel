/* Terminal Driver */
#include "terminal.h"
#include "lib.h"

/*define MACRO*/
#define KEYBOARD_TABLE_SIZE 88

/*define terminal buffer*/
static char terminal_buf[128]; 

/*define next_avail_index*/
static int32_t next_avail_index = 0; 

static int num_times_read_called = 0; 

/* 
 *   get_terminal_buf()
 *   DESCRIPTION: Returns a pointer to the terminal buffer  
 *   INPUTS: N/A
 *   OUTPUTS: N/A
 *   RETURN VALUE:  pointer to terminal buffer 
 */
char* get_terminal_buf(){
    return terminal_buf; 
}

/* 
 *   get_next_avail_index()
 *   DESCRIPTION: Returns next available index in the keyboard buffer 
 *   INPUTS: N/A
 *   OUTPUTS: N/A
 *   RETURN VALUE:  next_avail_index
 */
int32_t get_next_avail_index(){
    return next_avail_index; 
}

/* 
 *   terminal_open
 *   DESCRIPTION: Opens the terminal, nothing to be done for this function.
 *   INPUTS: N/A
 *   OUTPUTS: N/A
 *   RETURN VALUE: return 0 
 */
int32_t terminal_open (const uint8_t* filename){
    return 0; 
}

/* 
 *   terminal_read
 *   DESCRIPTION: Terminal reads the keyboard buffer (terminal_read_buf) and puts its content
 *                into the terminal buffer (terminal_buf). Only returns when ENTER key is 
 *                pressed or when terminal_buf is full. Puts newline char at the end of buffer
 *                before returning
 *   INPUTS: fd, terminal_read_buf, nbytes
 *   OUTPUTS: filled terminal_read_buf 
 *   RETURN VALUE: return 0 
 */
int32_t terminal_read(int32_t fd, void* terminal_read_buf, int32_t nbytes){
    /*check if the nbytes passed in to read is greater than the maximum terminal read buffer size*/
    num_times_read_called++;
    
    if(nbytes > 128){
        /*if so, sets the bytes to read to maximum of 128*/
        nbytes = 128;
    }

    /*obtain next available index with the getter*/
    next_avail_index=get_next_index(); 

    int i; 
    /*while the buffer is not full (128) and the ENTER (\n) is not pressed*/
    while(next_avail_index < nbytes && ((uint8_t*) terminal_read_buf)[next_avail_index] != '\n') {
        /*update the terminal read buffer to the keyboard buffer*/
        for (i = 0; i < nbytes; i++) {
            ((uint8_t*)terminal_read_buf)[i] = get_keys()[i];
        }
        /*obtain the next available index*/
        next_avail_index=get_next_index();
    }

    /*when the buffer is full (128) or the ENTER (\n) is pressed*/
    for (i = 0; i < 128; i++) {
        /*clear the terminal buffer, sets all terminal buffer entries to \0*/
        get_keys()[i] = '\0';
    }

    /*resets the next index to 0*/
    set_next_index(0);

    /*puts the newline char (\n) at the end of terminal buffer before returning*/
    ((uint8_t*)terminal_read_buf)[next_avail_index] = '\n';
    
    /*counter to access element in terminal read buffer*/
    i = 0;
    /*resets nbytes to 0*/
    nbytes = 0;

    /*checks how long the terminal read buffer is*/
    while(((uint8_t*)terminal_read_buf)[i] != '\n'){
        /*sets the nbytes to the length of the terminal read buffer*/
        nbytes++;
        /*increments counter by 1*/
        i++;
    }
    if(((uint8_t*)terminal_read_buf)[i] == '\n'){
        /*increment by 1 to account for the next line char*/
        nbytes++;
    }

    /*return number of bytes*/
    return nbytes;
}

/* 
 *   terminal_write
 *   DESCRIPTION: Terminal writes the number of chars of the provided in the argument
 *   INPUTS: fd, terminal_write_buf, nbytes
 *   OUTPUTS: ouputs terminal_read_buf's content to the screen
 *   RETURN VALUE: return 0 
 */
int32_t terminal_write(int32_t fd, const void* terminal_write_buf, int32_t nbytes){
    /*checks if the terminal buffer is null*/
    if((uint8_t*)terminal_write_buf==NULL){
        /*if so, return fail*/
        return -1; 
    }

    int i;
    int tab_flag = 0;
   /*checks if we are at the end of line and do tab properly*/
    if(nbytes>=76 && nbytes<80){
        for(i=76; i<80; i++){
            if(((uint8_t*)terminal_write_buf)[i]=='\t'){
                /*if there is a tab*/
                tab_flag=1;
            }
        }
    }
    /*checks if tab flag is 1*/
    if(tab_flag==1){
        /*prints nextline*/
        printf("\n");
    }
    /*traverse through nbytes to write*/
    for (i = 0; i < nbytes; i++) {
        /*prints terminal_write_buf's content */
        printf("%c",((uint8_t*) terminal_write_buf)[i]);

    }
    
    /*return the nbytes*/
    return nbytes;
}

/* 
 *   terminal_close
 *   DESCRIPTION: Closes the terminal, nothing to be done for this function.
 *   INPUTS: N/A
 *   OUTPUTS: N/A
 *   RETURN VALUE: return 0 
 */
int32_t terminal_close (int32_t fd){
    return 0; 
}
