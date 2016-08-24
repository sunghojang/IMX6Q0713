KDIR	:= /root/IMX6Q0713/kernel_imx
CROSS 	:= /opt/poky/1.5.1/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
obj-m := test.o
PWD 	:= $(shell pwd)
default:
	$(MAKE) -C $(KDIR) ARCH=arm CROSS_COMPILE=$(CROSS) SUBDIRS=$(PWD) modules


