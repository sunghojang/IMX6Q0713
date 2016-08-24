cmd_ipc/syscall.o := arm-poky-linux-gnueabi-gcc -Wp,-MD,ipc/.syscall.o.d  -nostdinc -isystem /opt/poky/1.5.1/sysroots/x86_64-pokysdk-linux/usr/lib/arm-poky-linux-gnueabi/gcc/arm-poky-linux-gnueabi/4.8.1/include -I/root/IMX6Q0713/kernel_imx/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/root/IMX6Q0713/kernel_imx/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/root/IMX6Q0713/kernel_imx/include/uapi -Iinclude/generated/uapi -include /root/IMX6Q0713/kernel_imx/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -fno-dwarf2-cfi-asm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(syscall)"  -D"KBUILD_MODNAME=KBUILD_STR(syscall)" -c -o ipc/.tmp_syscall.o ipc/syscall.c

source_ipc/syscall.o := ipc/syscall.c

deps_ipc/syscall.o := \
  /root/IMX6Q0713/kernel_imx/include/uapi/linux/unistd.h \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/unistd.h \
    $(wildcard include/config/aeabi.h) \
    $(wildcard include/config/oabi/compat.h) \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/uapi/asm/unistd.h \

ipc/syscall.o: $(deps_ipc/syscall.o)

$(deps_ipc/syscall.o):
