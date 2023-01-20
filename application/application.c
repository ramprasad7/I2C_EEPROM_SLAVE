/*application to write & read*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<errno.h>


#define MAX 256


char buf_write[MAX];
char buf_read[MAX];

int len=0;


int main(){
	int fd;
	
	fd = open("/dev/eeprom_slave",O_RDWR);
    if(fd < 0){
		perror("Failed to open");
		return -1;
	}
    while(1){
        int choice;
        printf("Enter: \n1.write\n2.read\n3.exit\n");
        scanf("%d",&choice);

        switch(choice){
            case 1:{
                printf("Enter data to be written: ");
                scanf("%s",buf_write);
                len = strlen(buf_write) + 1;
                if(write(fd,buf_write,len) < 0){
                    perror("Write failed");
                    return -1;
                }
                sleep(1);
                break;
            }

            case 2:{
                if(read(fd,buf_read,len) < 0){
                    perror("Read failed");
                    return -1;
                }
                printf("Read: %s\n",buf_read);
                break;                
            }

            case 3:{
                    printf("Exit....\n");
                    close(fd);
                    exit(0);
            }
            default:{
                printf("Invalid choice\n");
                break;
            }
        }
    }
    return 0;
}
