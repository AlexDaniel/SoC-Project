CC=$(CROSS_COMPILE)gcc

all:
	make -C ../../linux-digilent/ M=$(PWD) modules
	$(CC) generator_cli.c -o generator_cli

clean:
	make -C ../../linux-digilent/ M=$(PWD) clean
