cmd_drivers/input/misc/built-in.o :=  arm-poky-linux-gnueabi-ld -EL    -r -o drivers/input/misc/built-in.o drivers/input/misc/gpio_event.o drivers/input/misc/gpio_matrix.o drivers/input/misc/gpio_input.o drivers/input/misc/gpio_output.o drivers/input/misc/gpio_axis.o drivers/input/misc/keychord.o drivers/input/misc/mma8450.o drivers/input/misc/uinput.o drivers/input/misc/isl29023.o 