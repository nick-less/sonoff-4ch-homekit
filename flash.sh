#!/bin/sh
RTOS_SDK=../esp-open-rtos
UART=/dev/cu.SLAB_USBtoUART

esptool.py -p ${UART} --baud 921600 write_flash -fs 1MB -fm dout -ff 40m \
	0x0 "${RTOS_SDK}/bootloader/firmware_prebuilt/rboot.bin" 0x1000 "${RTOS_SDK}/bootloader/firmware_prebuilt/blank_config.bin" 0x2000 ./$1
	
