#Produce with the Kernel build system a file called lll-gpio-driver.o
obj-m += gpio-driver.o

#build system directory for linux kernel
KDIR = /lib/modules/$(shell uname -r)/build

#Sub make system, run build system in this folder to create .o file
all:
	make -C $(KDIR) M=$(shell pwd) modules


clean:
	make -C $(KDIR) M=$(shell pwd) clean