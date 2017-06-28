#! /bin/sh 

name="$1"
shift

arm-none-linux-gnueabi-gcc "$name".c -o "$name".app -L/PB-SDK/arm-*/sysroot/usr/lib -I/PB-SDK/arm-*/sysroot/usr/include -linkview -Os -Wall -fomit-frame-pointer -D__ARM__ -lfreetype -lz "$@"
