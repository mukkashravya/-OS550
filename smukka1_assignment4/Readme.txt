i have implemented character Device in charDevice.c
To compile and execute run the below steps:

1. copy charDevice.c and Makefile and do make
2. insert te character device using sudo insmod charDevice.ko
3. next check by running producer and consumer using commands below:
	sudo ./producer /dev/charDevice
	sudo ./consumer /dev/charDevice
	
	
4. size of the device is 100.

I have not changed the producer.c and consumer .c files.
I have tested with 2 producers and consumers and observed that producer is getting blocked when the buffer size is full 
and consumer gets blocked when buffer is empty. checked with 2 producers and consumers.