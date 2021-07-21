#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
int main()
{
	char input[256] = {0x0, };
	int choice;
	int i;
	int fd;
	char uid[256]= {0x0, };
	char command[256] = {0x0, };
	FILE *p = NULL;
	
	while(1){
		char kernelInfo[256] = {0x0, };
		char open_block_file[256] = {0x0, };
	    	char open_block_user[256] = {0x0, };
       		char kill_block_user[256] = {0x0, };
	
		fd= open("/proc/mousehole", O_RDWR);
        	if(fd <0){
            		printf("kernel is not installed\n");
                	return 0;
	    	}	
	
		printf("\nfunctions\n 1 : enable specific file block to specific user\n 2 : disable specific file block to specific user\n 3 : enable prevent specific user's process from kill\n 4 : disable prevent specific user's process form kill\n 5 : check open block info \n 6 : check kill block info \n 7 : exit program\n");
		printf("Choose funtion : ");
		scanf("%d", &choice);
		
		if(choice == 1){		
			printf("specific file name? : ");
			scanf("%s", input);

			strcat(open_block_file, "2");
			strcat(open_block_file, input);

			printf("specific user name? : ");
			scanf("%s", input);
			
			sprintf(command,"id -u %s", input);
	        	p = popen(command, "r");
			if(p<0){
				printf("User name can't find\n");
				continue;
			}
			fgets(uid, sizeof(uid), p);
	
			strcat(open_block_user, "3");
			strcat(open_block_user, uid);
		
			write(fd, "01", strlen("01"));
	        	write(fd, open_block_file, strlen(open_block_file));
			write(fd, open_block_user, strlen(open_block_user));
		}
		else if(choice == 2){
			write(fd, "00", strlen("00"));
		}
		else if(choice == 3){
	        	printf("specific user name? : ");
			scanf("%s", input);
			
			sprintf(command,"id -u %s", input);
	        	p = popen(command, "r");
			if(p<0){
				printf("user name can't find\n");
				continue;
			}
	        	fgets(uid, sizeof(uid), p);
			
			strcat(kill_block_user, "4");
			strcat(kill_block_user, uid);
			
			write(fd, "11", strlen("11"));
			write(fd, kill_block_user, strlen(kill_block_user)); 
		}
		else if(choice == 4){
			write(fd, "10", strlen("10"));
		}
		else if(choice == 5){
			write(fd, "5", strlen("5"));
        	}
		else if(choice == 6){
			write(fd, "6", strlen("6"));
        	}
		else if(choice == 7){
			printf("Good bye\n");
			break;
		}
		else{	
			printf("wrong input");
		}
		fd= open("/proc/mousehole", O_RDWR);
		read(fd, kernelInfo, 256);
        printf("Kernel Info : %s\n", kernelInfo);
	}
	return 0;
}
