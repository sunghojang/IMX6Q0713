cmd_drivers/leds/trigger/built-in.o :=  arm-poky-linux-gnueabi-ld -EL    -r -o drivers/leds/trigger/built-in.o drivers/leds/trigger/ledtrig-timer.o drivers/leds/trigger/ledtrig-heartbeat.o drivers/leds/trigger/ledtrig-gpio.o 
