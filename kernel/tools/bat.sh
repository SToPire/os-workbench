cd ..
make
cd tools
make
./mkfs 64 ../build/kernel-x86_64-qemu mntPoint
