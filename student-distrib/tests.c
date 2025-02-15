#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"
#include "keyboard.h"
#include "rtc.h"
#include "filesystem.h"
#include "syscall.h"
#include "syscall_testing.h"


#define PASS 1
#define FAIL 0


/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// CHECKPOINT 1 TESTS ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	//TEST_HEADER;
	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}

/*
 * NOTE ON REMAINDER OF TESTS: 
 *
 * Calling INT $(#) in assembly calls the interrupt associated with this certain number (#)
 *
 */

/* EXCEPTION: Divide By 0 */
int div_by_zero(){
	int a = 0;
	int b;
	b = 1/a;
	return 0;
}

/* EXCEPTION: Debug Error */
int debug_error(){
	asm volatile("int $1");
	return 0; 
}

/* EXCEPTION: Non-maskable Interrupt */
int non_mask_interr(){
	asm volatile("int $2");
	return 0; 
}

/* EXCEPTION: Breakpoint */
int breakpoint(){
	asm volatile("int $3");
	return 0; 
}

/* EXCEPTION: Overflow */
int overflow(){
	asm volatile("int $4");
	return 0;  
}

/* EXCEPTION: Bound Range Exceeded */
int bound_range_exceeded(){
	asm volatile("int $5");
	return 0; 
}

/* EXCEPTION: Invalid Opcode */
int invalid_op(){
	asm volatile("int $6");
	return 0; 
}

/* EXCEPTION: Device Not Available */
int device_not_avail(){
	asm volatile("int $7");
	return 0; 
}

/* EXCEPTION: coprocessor_seg_overrun */
int coprocessor_seg_overrun(){
	asm volatile("int $9");
	return 0; 
}

/* EXCEPTION: x87_floating */
int x87_floating(){
	asm volatile("int $16");
	return 0; 
}

/* EXCEPTION: machine_check */
int machine_check(){
	asm volatile("int $18");
	return 0; 
}

/* EXCEPTION: SIMD_floating */
int SIMD_floating(){
	asm volatile("int $19");
	return 0; 
}

/* ERROR CODE: double_fault */
int double_fault(){
	asm volatile("int $8");
	return 0; 
}

/* ERROR CODE: invalid_tss */
int invalid_tss(){
	asm volatile("int $10");
	return 0; 
}

/* ERROR CODE: seg_not_present */
int seg_not_present(){
	asm volatile("int $11");
	return 0; 
}

/* ERROR CODE: stack_seg_fault */
int stack_seg_fault(){
	asm volatile("int $12");
	return 0; 
}

/* ERROR CODE: general_protection_fault */
int general_protection_fault(){
	asm volatile("int $13");
	return 0; 
}

/* ERROR CODE: page_fault */
int page_fault(){
	asm volatile("int $14");
	return 0; 
}

/* ERROR CODE: align_check */
int align_check(){
	asm volatile("int $17");
	return 0; 
}



/* PAGING TEST */
// Dereferences different addresses with paging enabled 
int page_test(){
	int* ptr =  NULL;					
	int* ptr1 = (int *)(-1);
	*ptr = 0x1;
	*ptr1 = 0x2;
	return 0; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// CHECKPOINT 1 TESTS ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// CHECKPOINT 2 TESTS ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* KEYBOARD AND TERMINAL DRIVER TEST */
void terminal_test(){
	char terminal_buf[128];
	(void)terminal_read(NULL, terminal_buf, 128);
	(void)terminal_write(NULL, terminal_buf, get_next_avail_index());
}


/* RTC DRIVER TEST WITH VARIYING VALID FREQUENCIES */
void rtc_test(){
	int32_t freq ;
	int i;
	int nbytes= 32; 						// nbytes will not be used (Just a random number)
	const uint8_t rtc_filename[10]= "RTC"; 	// Filename will not be used
	clear(); 

	//Test Individual Frequencies
	//void* buf=(int *)2;
	//rtc_write(fd, buf, nbytes);

	int32_t fd=rtc_open(rtc_filename); 

	// Testing all possible frequencies
	for(freq=2; freq<=1024; freq*=2 ){
		void* buf=(int *)freq;
		for(i=0; i<freq; i++){
			rtc_read(fd, buf, nbytes); 		// Prints right after a interrupt is fired

		}	
		rtc_write(fd, buf, nbytes); 		// rtc handler prints that a interrupt has occured
	}
	cli(); 									// Stop the rtc frequency test
}


/* RTC DRIVER TEST TESTING VALID/INVALID FREQUENCIES */
void rtc_bad_hz_test(int freq){
	clear();
	const uint8_t rtc_filename[10]= "RTC";
	int32_t fd=rtc_open(rtc_filename);	
	int nbytes= 32;				// Open the RTC
	void* buf=(int *)freq;
	if(rtc_write(fd, buf, nbytes)==0){
		printf("                           %d Hz is a power of 2!\n                           Changing RTC rate to %d Hz!\n", freq, freq);
	}
	else{
		printf("                           %d Hz is NOT a power of 2!\n                           Keeping RTC rate at 2 Hz!\n", freq);
	}

}


/* RTC OPEN TEST */
void rtc_open_test(){
	clear();
	const uint8_t rtc_filename[10]= "RTC";
	rtc_open(rtc_filename);

}


/* FILE OPEN/READ/CLOSE TEST FOR TXT FILES */
void file_test_f(){
	clear(); 
	/* Code below is used to test the individual helper functions for the File Driver */
		
		/* Testing read_dentry_by_index */
			// dentry_t dentry;
			// (void) read_dentry_by_index (2, &dentry);
			// printf("%d ", dentry.file_type);
			// printf("%s ", dentry.file_name);
			// printf("%d ", dentry.inode_num);
		
		/* Testing read_dentry_by_name */
			// dentry_t dentry;
			// const uint8_t name[32] = "pingpong";
			// (void) read_dentry_by_name (name, &dentry);
			// printf("%d ", dentry.file_type);
			// printf("%s ", dentry.file_name);
			// printf("%d ", dentry.inode_num);

		/* Testing read_data */
			// uint8_t filebuf[20];
			// int rv = read_data(38,0, (uint8_t*)filebuf, 20);	//inode = 38 goes with frame0.txt
			// //printf("%d",rv);
			// int i=0;
			// for(i=0; i<20;i++){
			// 	printf("%c",filebuf[i]);
			// }

		/* Testing for file_open, file_read and file_close*/
			//const uint8_t name[32] = "verylargetextwithverylongname.txt";
			// const uint8_t name[32] = "frame0.txt";
			// //const uint8_t name[32] = "frame1.txt";
			// int32_t fd_num = file_open (name);
			// file_desc_t* fd_descriptor = get_fd_info(fd_num);
			// int32_t index_node = fd_descriptor->inode_num;
			// inode_t* inode = get_inode_info(index_node);
			// int length = inode->length_bytes;
			// // printf("%d ", fd_descriptor->inode_num);
			// // printf("%d ", fd_descriptor->file_pos);
			// // printf("%d ", fd_descriptor->flags);
			// uint8_t filebuf[length];
			// (void) file_read (fd_num, filebuf, length);
			// int i;
			// for(i=0; i<length;i++){
			// 	printf("%c",filebuf[i]);
			// }
			// (void) file_close(fd_num);
}


/* FILE OPEN/READ/CLOSE TEST FOR EXECUTABLES */
void file_test_ex(){
	clear(); 

	/* Testing for file_open, file_read and file_close*/
		//const uint8_t name[32] = "pingpong";
		//const uint8_t name[32] = "hello";
		//const uint8_t name[32] = "ls";
		//const uint8_t name[32] = "fish";
		// int32_t fd_num = file_open (name);
		// file_desc_t* fd_descriptor = get_fd_info(fd_num);
		// int32_t index_node = fd_descriptor->inode_num;
		// inode_t* inode = get_inode_info(index_node);
		// int length = inode->length_bytes;
		// printf("%d ", fd_descriptor->inode_num);
		// printf("%d ", fd_descriptor->file_pos);
		// printf("%d ", fd_descriptor->flags);
		// uint8_t filebuf[length];
		// (void) file_read (fd_num, filebuf, length);
		// int i;
		// //printf("length %d\n", length);
		// int counter = 0; 
		// for(i=0; i<length;i++){
		// 	if(filebuf[i]!='\0'){
		// 		printf("%c",filebuf[i]);
		// 		counter++;
		// 	}
				
		// 	if(counter%80==0 && i!=0){
		// 		printf("\n");
		// 	}
		// }
		// (void) file_close(fd_num);
}


/* DIRECTORY OPEN/READ/CLOSE TEST */
void directory_test(){
	clear();
	uint8_t file_name_buf[200];
	int file_name_length = 32;
	int file_type_length = 1;
	int file_size_length = 7;
	uint8_t directory[32] = ".";
	int32_t fd = dir_open (directory);
	int i;
	while(dir_read(fd, file_name_buf, file_name_length) != -1){

		printf("file_name: ");
		for(i=0; i<file_name_length;i++){
			printf("%c",file_name_buf[i]);
		}

		printf(", file_type: ");
		for(i=0; i<file_type_length;i++){
			// 32 as the character take 32 characters
			printf("%c",file_name_buf[32+i]);
		}

		printf(", file_size:");
		for(i=0; i<file_size_length;i++){
			// 33 as the data before take 33 char in total
			printf("%c",file_name_buf[33+i]);
		}
		
		printf("\n");
	}
	(void) dir_close (fd);

	// !!! need to reset counter in dir_read if we want to perform ls (terminal command) multiple time !!!
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// CHECKPOINT 2 TESTS ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// CHECKPOINT 3 TESTS ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* SYSTEM CALL TESTS (execute, halt, open, close, read, write)*/
int system_call_test(){
	//int num = -1;
	clear();

	// Testing EXECUTE on testprint (valid eax value) (travel through IDT, sycall_handler)
		// char bla[32] = "testprint";
		// num = 2; 										// 2 for execute
		// printf("testing system call execute \n");
		// printf("input command addr is %x\n", bla);
		// syscall_parse(num, bla);
		// printf("testing system call execute end \n");

	// Testing EXECUTE on testprint (invalid eax value) (travel through IDT, sycall_handler)
		// num = -1; 
		// printf("testing system call bad num \n");
		// printf("input command addr is %x\n", bla);
		// syscall_parse(num, bla);
		// printf("testing system call bad num end \n");

	// Testing EXECUTE on counter (go directly into system_execute)
		// system_execute("counter");

	// Testing EXECUTE on shell (go directly into system_execute)
		// system_execute("shell");

	// while(1){
	// 	system_execute((const uint8_t*)("shell"));
	// }

	return 0; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////// CHECKPOINT 3 TESTS ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/* Test suite entry point */
 void launch_tests(){

	/* CHECKPOINT 3 TESTS */

	/*Syscall Test #1: system_call_test()*/
	(void) system_call_test(); 


	/* CHECKPOINT 2 TESTS */

	/*RTC Test #1: testing rtc_open*/
	//rtc_open_test();

	/*RTC Test #2: testing rtc_read & rtc_freq*/
	//rtc_test();

	/*RTC Test #3: testing rtc_write with valid and invalid frequencies */
	//rtc_bad_hz_test(32);

	/*File Test*/
	//file_test_f(); 
	//file_test_ex(); 

	/*Directory Test*/
	// directory_test(); 

	/*Terminal Test*/
	// while(1){
	// 	terminal_test();
	// }
	

	/* CHECKPOINT 1 TESTS */

	/*IDT Test #1: align_check()*/
 	//TEST_OUTPUT("IDT Test", align_check());
	
	/*IDT Test #2: double_fault()*/
	//(void) double_fault(); 
	
	/*IDT Test #3: div_by_zero()*/
	//(void) div_by_zero(); 
		
	/*Paging Test #1: page_fault()*/
	//(void) page_test(); 
 	
 }

