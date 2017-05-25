#!/bin/sh

#===============================================================================
CURRENT_UTILITY_DIR=$(pwd)
echo "..."
echo $CURRENT_UTILITY_DIR
RAMFILENAME="./build/ram_all.bin"
echo $RAMFILENAME
#RAMFILENAME="ram_2.bin"
GDBSCPTFILE="./vendor/component/soc/realtek/8195a/misc/gcc_utility/rtl_gdb_flash_write.txt"

OS=uname
#===============================================================================
#get file size
RAM_FILE_SIZE=$(stat -f%z $RAMFILENAME)
RAM_FILE_SIZE_HEX=`echo "obase=16; $RAM_FILE_SIZE"|bc`

echo "size "$RAM_FILE_SIZE" --> 0x"$RAM_FILE_SIZE_HEX

echo "set \$RamFileSize = 0x$RAM_FILE_SIZE_HEX" > fwsize.gdb
exit
