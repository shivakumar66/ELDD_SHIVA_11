obj-m :=exam_prob.o

KERNDIR=/lib/modules/$(shell uname -r)/build
PWD:=$(shell pwd)

default: 
	$(MAKE) -C $(KERNDIR) M=$(PWD) modules
	gcc -o read read.c
	gcc -o write write.c 

clean:
	$(MAKE) -C $(KERNDIR) M=$(PWD) clean 
	rm read write
         
