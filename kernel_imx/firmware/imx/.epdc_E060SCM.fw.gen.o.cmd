cmd_firmware/imx/epdc_E060SCM.fw.gen.o := arm-poky-linux-gnueabi-gcc -Wp,-MD,firmware/imx/.epdc_E060SCM.fw.gen.o.d  -nostdinc -isystem /opt/poky/1.5.1/sysroots/x86_64-pokysdk-linux/usr/lib/arm-poky-linux-gnueabi/gcc/arm-poky-linux-gnueabi/4.8.1/include -I/root/IMX6Q0713/kernel_imx/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/root/IMX6Q0713/kernel_imx/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/root/IMX6Q0713/kernel_imx/include/uapi -Iinclude/generated/uapi -include /root/IMX6Q0713/kernel_imx/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian  -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float         -c -o firmware/imx/epdc_E060SCM.fw.gen.o firmware/imx/epdc_E060SCM.fw.gen.S

source_firmware/imx/epdc_E060SCM.fw.gen.o := firmware/imx/epdc_E060SCM.fw.gen.S

deps_firmware/imx/epdc_E060SCM.fw.gen.o := \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \

firmware/imx/epdc_E060SCM.fw.gen.o: $(deps_firmware/imx/epdc_E060SCM.fw.gen.o)

$(deps_firmware/imx/epdc_E060SCM.fw.gen.o):
