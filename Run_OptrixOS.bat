@echo off

qemu-system-x86_64 -drive format=raw,file=disk.img -drive format=raw,file=resources.img
