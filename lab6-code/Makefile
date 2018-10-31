#obj-m += printhex.o
#obj-m += my_hexdump.o
#obj-m += mydumpstack.o
obj-m += mysyslog.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
