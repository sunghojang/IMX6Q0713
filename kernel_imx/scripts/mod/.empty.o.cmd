cmd_scripts/mod/empty.o := arm-poky-linux-gnueabi-gcc -Wp,-MD,scripts/mod/.empty.o.d  -nostdinc -isystem /opt/poky/1.5.1/sysroots/x86_64-pokysdk-linux/usr/lib/arm-poky-linux-gnueabi/gcc/arm-poky-linux-gnueabi/4.8.1/include -I/root/IMX6Q0713/kernel_imx/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/root/IMX6Q0713/kernel_imx/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/root/IMX6Q0713/kernel_imx/include/uapi -Iinclude/generated/uapi -include /root/IMX6Q0713/kernel_imx/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -fno-dwarf2-cfi-asm -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a -msoft-float -Uarm -Wframe-larger-than=1024 -fno-stack-protector -Wno-unused-but-set-variable -fomit-frame-pointer -fno-var-tracking-assignments -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(empty)"  -D"KBUILD_MODNAME=KBUILD_STR(empty)" -c -o scripts/mod/.tmp_empty.o scripts/mod/empty.c

source_scripts/mod/empty.o := scripts/mod/empty.c

deps_scripts/mod/empty.o := \

scripts/mod/empty.o: $(deps_scripts/mod/empty.o)

$(deps_scripts/mod/empty.o):
