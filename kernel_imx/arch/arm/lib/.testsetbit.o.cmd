cmd_arch/arm/lib/testsetbit.o := arm-poky-linux-gnueabi-gcc -Wp,-MD,arch/arm/lib/.testsetbit.o.d  -nostdinc -isystem /opt/poky/1.5.1/sysroots/x86_64-pokysdk-linux/usr/lib/arm-poky-linux-gnueabi/gcc/arm-poky-linux-gnueabi/4.8.1/include -I/root/IMX6Q0713/kernel_imx/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/root/IMX6Q0713/kernel_imx/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/root/IMX6Q0713/kernel_imx/include/uapi -Iinclude/generated/uapi -include /root/IMX6Q0713/kernel_imx/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian  -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float         -c -o arch/arm/lib/testsetbit.o arch/arm/lib/testsetbit.S

source_arch/arm/lib/testsetbit.o := arch/arm/lib/testsetbit.S

deps_arch/arm/lib/testsetbit.o := \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/sparse/rcu/pointer.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
    $(wildcard include/config/kprobes.h) \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/linkage.h \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/assembler.h \
    $(wildcard include/config/cpu/feroceon.h) \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/cpu/use/domains.h) \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/ptrace.h \
    $(wildcard include/config/arm/thumb.h) \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/uapi/asm/ptrace.h \
    $(wildcard include/config/cpu/endian/be8.h) \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/hwcap.h \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/uapi/asm/hwcap.h \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/domain.h \
    $(wildcard include/config/io/36.h) \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/opcodes-virt.h \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/opcodes.h \
    $(wildcard include/config/cpu/endian/be32.h) \
  arch/arm/lib/bitops.h \
  /root/IMX6Q0713/kernel_imx/arch/arm/include/asm/unwind.h \
    $(wildcard include/config/arm/unwind.h) \

arch/arm/lib/testsetbit.o: $(deps_arch/arm/lib/testsetbit.o)

$(deps_arch/arm/lib/testsetbit.o):
