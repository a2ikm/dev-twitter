TARGET:= twitter.ko
MODNAME:= twitter

all: ${TARGET}

twitter.ko: twitter.c
	make -C /usr/src/kernels/`uname -r` M=`pwd` V=1 modules

clean:
	make -C /usr/src/kernels/`uname -r` M=`pwd` V=1 clean

install: all
	/sbin/insmod ${TARGET}
	mknod /dev/${MODNAME} c `cat /proc/devices | tee | grep ${MODNAME} | cut -d" " -f 1` 0
	chmod 666 /dev/${MODNAME}

uninstall:
	/sbin/rmmod ${MODNAME}
	rm -f /dev/${MODNAME}

obj-m:= twitter.o

clean-files := *.o *.ko *.mod.[co] *~
