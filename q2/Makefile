obj-m :=driver2.o

KERNDIR=/lib/modules/$(shell uname -r)/build
PWD:=$(shell pwd)

default: 
	$(MAKE) -C $(KERNDIR) M=$(PWD) modules
	gcc -o app app.c

clean:
	$(MAKE) -C $(KERNDIR) M=$(PWD) clean 
	rm app
         
