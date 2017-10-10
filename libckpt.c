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
struct map_data maps[1000];
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
	printf("inside checkpoint handler");
	do_checkpoint();
	//write_register_data();	
	return;
}

void sigusr1_handler(int signum)
{
        printf("insid sigusr");
        if (signum == SIGUSR2)
        {
                printf("Received SIGUSR1!\n");
		checkpoint_handler();
        }
}

 __attribute__ ((constructor))
            void myconstructor() {
		printf("running constructor\n");
              signal(SIGUSR2, sigusr1_handler);
            }


/* Read decimal number, return value and terminating character */
long mtcp_readhex (char adr[20])
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
	//write_jmpbuf_to_ckpt_header(myjmpbuf);
	  //      printf("2");
	
    	int i, j, size_struct;

	FILE *file = fopen("/proc/self/maps", "r");
    	char entries[1000];
	int size;
	
	struct map_data sizes;
    	if (!file) {
        	perror("error reading file");
    	}

    	char line [line_size];
	
    	while (fgets(line, 1000, file))
    	{
		char test, substr[32], low_value1, high_value1;
		int n;
		int r, w, ex;
		// Parse the Low address
		if (sscanf(line, "%31[^:]%n", substr, &n) == 1){
			        printf("-------------------\n");
			printf("%s", substr);
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
		low_value  = strtoll(low, NULL, 16);
		high_value = strtoll(high, NULL, 16);	
		
		low_value1  = mtcp_readhex(low);
                high_value1 = mtcp_readhex(high);
		
		//test = strtoll(low_value1, NULL, 16);
		//printf("%s\n", test);
	//	printf("data values\n");
	//	printf("%ld\t", low_value1);
          //      printf("%ld\t", high_value1);
            //    printf("values end\n");
		
		
		maps[counter].start_addr = low_value;
		maps[counter].end_addr = high_value;
		maps[counter].isContext = 0;
		maps[counter].read = r;
		maps[counter].write = w;
		maps[counter].execute = ex;
		counter = counter+1;
		//print sscanf(mapbuf,"%"KLF"x-%"KLF"x %31s %Lx %x:%x %Lu", &start, &end, flags, &file_offset, &dev_major, &dev_minor, &inode);

		//if (sscanf(line, "%63[^:]%n", substr, &n) == 1)
        //	{	
	//		s1 = strtok(substr, " ");
	//		s2 = strtok(NULL, " ");
	//	}
	}
}

void do_checkpoint(){
	printf("1");
	
	int ckpt_image = open("myckpt", O_RDWR|O_CREAT, 0777);
	        printf("inside do checkpoint");
	read_memory_layout();
	int records = 0;
	unsigned long long low_value, high_value;
	while(records!=counter-1)
		{
			write(ckpt_image, &maps[records], sizeof(maps[records]));
			low_value = maps[records].start_addr;
                        high_value = maps[records].end_addr;
			write(ckpt_image, (char *) low_value, high_value-low_value);
			records++;
		}
	maps[records+1].isContext = 1;
	write(ckpt_image, &maps[records+1], sizeof(maps[records+1]));
	//write_register_data();
	getcontext(&context_data); 
	write(ckpt_image, &context_data, sizeof(context_data));
	close(ckpt_image);
}

int main(){
	do_checkpoint();
//	signal(12, sigusr1_handler);
	return 0;
}



