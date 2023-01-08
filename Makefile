obj-m += hru.o
hru-objs := module/hru.o module/grunt.o

all:
	make build
	make replug

build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

replug:
	-rmmod hru
	insmod hru.ko
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
