/*application to read*/
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
	fd = open("/dev/eeprom_slave",O_RDWR);


	if(fd < 0){
		printf("failed to open\n");
		return -1;
	}

	if(read(fd,buf,strlen(buf)+1) < 0){
		printf("failed to read\n");
		return -1;
	}

    printf("%s\n",buf);
	close(fd);
	return 0;
}
