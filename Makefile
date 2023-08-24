obj-m += hru.o
hru-objs := module/hru.o module/grunt.o
EXTRA_CFLAGS += -I$(src)/module

all: build replug clean

build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

replug:
	-rmmod hru
	insmod hru.ko
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

vscode:
	make -C $(PWD)/.vscode M=$(PWD)/.vscode all
