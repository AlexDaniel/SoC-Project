CC=$(CROSS_COMPILE)gcc

all:
	make -C ../../linux-digilent/ M=$(PWD) modules
	$(CC) ZedboardOLED.c networktoip_cli.c -o networktoip_cli

clean:
	make -C ../../linux-digilent/ M=$(PWD) clean
