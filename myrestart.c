#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <errno.h>
#include <stdint.h>
#include <setjmp.h>
#include <signal.h>
#include <ucontext.h>

char ckpt_image[1000];
typedef unsigned long long ll;
unsigned long long stk_l_value, stk_h_value;

struct map_data {
        long long start;
        long long  end;
        int read;
        int write;
        int execute;
        int isContext;;
}mapped_data;;

struct map_data m_d;
void read_checkpoint_data(){
	ucontext_t context_d;
	bool read_registers = 0;
	        int fd = open(ckpt_image, O_RDONLY);
	while(read_registers==0){
		int data = read(fd, &mapped_data, sizeof(mapped_data));
		if(data == -1){
                	printf("error while reading the checkpint image");
                        exit(0);
                }
		size_t size = mapped_data.end - mapped_data.start;
		printf("%d", mapped_data.isContext);
		printf("mapping data now");
		if(mapped_data.isContext == 0 && mapped_data.start != mapped_data.end){
			printf("%d", mapped_data.isContext);
			void *start = (void *)mapped_data.start;
			if(mapped_data.start != 0){
			//mapped_data.read|mapped_data.write|mapped_data.execute
               	  	int res = munmap((void *)mapped_data.start, size);		
			if(mmap(start, size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED, -1, 0) == MAP_FAILED){
				printf("Error in mapping memory using mmap.");
				exit(0);
			}
			int read_res = read(fd, start, size);		
			if(read_res != size){
				perror("failed");
				exit(0);
			}
			}
		}
		else{
			read_registers = 1;
			int i=0;
			int result = read(fd, &context_d, sizeof(context_d));
			if(result != sizeof(context_d)){
				printf("error in reading context from checkpoint data");
			}
			int res = setcontext(&context_d);
			//int res = longjump(buffer_data, 2);
			if(res == -1){
				perror("Might be an error in setting context.");
				exit(0);
			}
			break;
		}	
	}
	close(fd);
}

//find the stack address entry from the proc/self/maps 
void find_process_stack_addr(){
	char stack_start[20], stack_end[20];
	FILE *file = fopen("/proc/self/maps", "r");
	int stack_segment = 0;
	if(!file){
		perror("error opening the proc/self/maps file");
	}
	char line[1000];
	while(fgets(line, 1000, file)){	
		int i=0;
		char *seg_info;
		//check if the line has 's' in it just after '['. This is a loose way to find the stack. Should be improved
		for(i = 0; line[i] != '\0'; i++){
			if(line[i]=='['){
				int count = 0;
				if(line[i+1]=='s'){
					stack_segment = 1;
					break;
				}
			}
		}
		if(stack_segment==1){
			int i=0;
			int j =0;
			for(i=0, j=0;line[i]!='-';i++, j++){
				stack_start[j] = line[i];
			}
			for (j = 0, i++; line[i] != ' '; i++, j++)
                                stack_end[j]= line[i];
		break;
		}
	}
	
	//strtoll for C99 and Posix also works to read hex values
	stk_l_value = strtoll(stack_start, NULL, 16);
        stk_h_value= strtoll(stack_end, NULL, 16);
}

void* map_stack_data_to_new_addr(void *new_stack){
	size_t n = stk_h_value - stk_l_value;
	return memcpy(new_stack, (void *)stk_l_value, n);
}

void unmap_stack_segment(){
	size_t n = stk_h_value - stk_l_value;
	size_t n1 = 1000;
	//munmap((void *)stk_l_value, n);
}


void restore_checkpoint_data(){
	//creating a new stack segment before moving the stack of curent program so that its stack segment does not collide with the addresses in the checkpoint image	
	unsigned long long low_adr, high_adr;
	char *st_adr = "6700000";
	low_adr = strtoll(st_adr, NULL, 16);
	
	void* stack_ptr = mmap((void *)low_adr, 400000, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED, -1, 0); 
	if(stack_ptr == MAP_FAILED){
		printf("Error");
		exit(0);
	}
	//pointing the stack pointer of current program to the new memory segment
	find_process_stack_addr();
	//mapping the 
	void *dest_stack = map_stack_data_to_new_addr(stack_ptr);
	asm volatile ("mov %0, %%rsp;" : : "g" (&stack_ptr) : "memory");
	unmap_stack_segment();
	read_checkpoint_data();
} 

int main(int argc, char *argv[]){
	if(argc<2){
		printf("Please provide checkpoint image");
	} 
	strcpy(ckpt_image, argv[1]);
	int fd = open(ckpt_image, O_RDONLY);
	if(fd == -1){
		perror("error while opening checkpoint file.");
	}
	restore_checkpoint_data();
}
