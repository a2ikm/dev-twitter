TARGET:= twitter.ko

all: ${TARGET}

twitter.ko: twitter.c
	make -C /usr/src/kernels/`uname -r` M=`pwd` V=1 modules

clean:
	make -C /usr/src/kernels/`uname -r` M=`pwd` V=1 clean

obj-m:= twitter.o

clean-files := *.o *.ko *.mod.[co] *~
