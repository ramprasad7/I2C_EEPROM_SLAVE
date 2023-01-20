/*application to write*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>



int main(){
	int fd;
	char buf[1024];
	char buf1[1024] = {0};
	fd = open("/dev/eeprom_slave",O_RDWR);


	if(fd < 0){
		printf("failed to open\n");
		return -1;
	}

	printf("enter a string: ");
	scanf("%s",buf);
	if(write(fd,buf,strlen(buf)+1) < 0){
		printf("failed to write\n");
		return -1;
	}
	sleep(1);
	close(fd);
	return 0;
}
