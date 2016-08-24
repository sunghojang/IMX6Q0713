cmd_arch/arm/boot/dts/imx6q-sabresd.dtb := arm-poky-linux-gnueabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.imx6q-sabresd.dtb.d.pre.tmp -nostdinc -I/root/IMX6Q0713/kernel_imx/arch/arm/boot/dts -I/root/IMX6Q0713/kernel_imx/arch/arm/boot/dts/include -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.imx6q-sabresd.dtb.dts.tmp arch/arm/boot/dts/imx6q-sabresd.dts ; /root/IMX6Q0713/kernel_imx/scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/imx6q-sabresd.dtb -b 0 -i arch/arm/boot/dts/  -d arch/arm/boot/dts/.imx6q-sabresd.dtb.d.dtc.tmp arch/arm/boot/dts/.imx6q-sabresd.dtb.dts.tmp ; cat arch/arm/boot/dts/.imx6q-sabresd.dtb.d.pre.tmp arch/arm/boot/dts/.imx6q-sabresd.dtb.d.dtc.tmp > arch/arm/boot/dts/.imx6q-sabresd.dtb.d

source_arch/arm/boot/dts/imx6q-sabresd.dtb := arch/arm/boot/dts/imx6q-sabresd.dts

deps_arch/arm/boot/dts/imx6q-sabresd.dtb := \
  arch/arm/boot/dts/imx6q.dtsi \
  arch/arm/boot/dts/imx6q-pinfunc.h \
  arch/arm/boot/dts/imx6qdl.dtsi \
  arch/arm/boot/dts/skeleton.dtsi \
  /root/IMX6Q0713/kernel_imx/arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \
  arch/arm/boot/dts/imx6qdl-sabresd.dtsi \
    $(wildcard include/config/mango/touch/ft6x36/5inch.h) \

arch/arm/boot/dts/imx6q-sabresd.dtb: $(deps_arch/arm/boot/dts/imx6q-sabresd.dtb)

$(deps_arch/arm/boot/dts/imx6q-sabresd.dtb):
