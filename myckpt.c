#include<stdio.h>
#include <fcntl.h>
#include<signal.h>
#include<unistd.h>
#include <setjmp.h>
#include <signal.h>
#include <ucontext.h>


struct map_data {
    	long long start_addr;
    	long long  end_addr;
	int read;
	int write;
	int execute;
    	int isContext;;
};

char low[15], high[15];
unsigned long long low_value, high_value;
ucontext_t  context_data;
int counter = 0;
//struct map_data maps[1000];
struct map_data maps;

jmp_buf myjmpbuf;
int line_size = 1000;

void do_checkpoint();

int  write_jmpbuf_to_ckpt_header(){
	int res = setjmp(myjmpbuf);
	if(res!=0)
		printf("error");
	printf("%d\n", res);
	printf("%s\n", myjmpbuf);
	return res;
}


//void write_register_data(){
//	int res = write_jmpbuf_to_ckpt_header();
//	if(res!=0)
//		printf("Error storing the registers.");
//	getcontext(&maps[counter].context_d);
	//(maps[counter].reg_buffer)[1] = myjmpbuf[1];
//	maps[counter].isContext = 1;
	//write(ckpt_image, &maps[counter], sizeof(maps[counter]));
	//int a = getcontext(&s);
	//if(!(access("context_ckpt", F_OK) != -1)){
	//	int context_file = open("context_ckpt", O_RDWR|O_CREAT, 0777);
	//	int bytes_written = write(context_file, &s, sizeof(s));
	//	close(context_file);
	//}
//	return;
//}

void checkpoint_handler(){
	do_checkpoint();
	//write_register_data();	
	return;
}

void sigusr1_handler(int signum)
{
        if (signum == SIGUSR2)
        {
                printf("Received SIGUSR2!\n");
		checkpoint_handler();
        }
}

 __attribute__ ((constructor))
            void myconstructor() {
              signal(SIGUSR2, sigusr1_handler);
            }


/* Read decimal number, return value and terminating character */
unsigned long long mtcp_readhex (char adr[20])
{
	char c;
  	unsigned long int v;
 	v = 0;
	int i=0;
	
  	while (1) {
    		c = adr[i];
      		if ((c >= '0') && (c <= '9')) c -= '0';
    		else if ((c >= 'a') && (c <= 'f')) c -= 'a' - 10;
    		else if ((c >= 'A') && (c <= 'F')) c -= 'A' - 10;
    		else break;
    		v = v * 16 + c;
		i++;
  	}
  return v;
}


void read_memory_layout(){
    	int i, j, size_struct;

	FILE *file = fopen("/proc/self/maps", "r");
    	char entries[1000];
	int size;
	int ckpt_image = open("myckpt", O_RDWR|O_CREAT|O_TRUNC, 0777);
    	if (!file) {
        	perror("error reading file");
    	}
	unsigned long long test = 1342435332;
    	char line [line_size];
    	while (fgets(line, 1000, file))
    	{
		char test, substr[32];
		unsigned long long low_value1, high_value1;
		int n;
		int r, w, ex;
		// Parse the Low address
		if (sscanf(line, "%31[^:]%n", substr, &n) == 1){
		//	        printf("-------------------\n");
		//	printf("%s", substr);
			for (i = 0, j = 0; substr[i]!= '-'; i++, j++)
		 		low[j]= substr[i];
			low[j]='\0';
		// Parse the High address
 	        	for (j = 0, i++; substr[i] != ' '; i++, j++)	
	                	high[j]= substr[i];
               	 	high[j]='\0'; 
			if(substr[++i]=='r')
				r=1;
			if(substr[++i]=='w')
				w=1;
			if(substr[++i]=='x')
				ex=1;
		}

		// Convert addresses to hex
		low_value1  = mtcp_readhex(low);
                high_value1 = mtcp_readhex(high);
		
		maps.start_addr = low_value1;
		maps.end_addr = high_value1;
		maps.isContext = 0;
		maps.read = r;
		maps.write = w;
		maps.execute = ex;
		counter = counter+1;

		int w_res = write(ckpt_image, &maps, sizeof(maps));
		int w_res_d = write(ckpt_image, (char *) low_value1, high_value1-low_value1);
	}
	maps.isContext = 1;
        write(ckpt_image, &maps, sizeof(maps));
        getcontext(&context_data);
        write(ckpt_image, &context_data, sizeof(context_data));
        close(ckpt_image);
}

void do_checkpoint(){
	        read_memory_layout();
}



