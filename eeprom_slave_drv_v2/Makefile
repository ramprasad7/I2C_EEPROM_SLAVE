obj-m := i2c_eeprom-slave.o  
 
host:
	make -C /lib/modules/$(shell uname -r)/build  M=$(shell pwd) modules
 


clean:
	 make -C /lib/modules/$(shell uname -r)/build  M=$(shell pwd) clean
