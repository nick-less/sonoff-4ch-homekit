#!/bin/sh
RTOS_SDK=../esp-open-rtos
#UART=/dev/cu.wchusbserial1420
UART=/dev/cu.SLAB_USBtoUART
#UART=/dev/cu.SLAB_USBtoUART16

#esptool.py -p ${UART} --baud 921600 write_flash -fs 2MB -fm qio -ff 40m \
#	0x0 "${RTOS_SDK}/bootloader/firmware_prebuilt/rboot.bin" 0x1000 "${RTOS_SDK}/bootloader/firmware_prebuilt/blank_config.bin" 0x2000 ./$1

esptool.py -p ${UART} --baud 921600 write_flash -fs 1MB -fm dout -ff 40m \
	0x0 "${RTOS_SDK}/bootloader/firmware_prebuilt/rboot.bin" 0x1000 "${RTOS_SDK}/bootloader/firmware_prebuilt/blank_config.bin" 0x2000 ./$1
	
#esptool.py -p ${UART} --baud 921600 write_flash -fs 2MB -fm dio -ff 40m \
#	0x1000 "${RTOS_SDK}/bootloader/firmware_prebuilt/rboot.bin" 0x4000 "${RTOS_SDK}/bootloader/firmware_prebuilt/blank_config.bin" 0x10000 ./$1
	
