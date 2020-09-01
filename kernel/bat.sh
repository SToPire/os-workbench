make
cd tools
make
./mkfs 64 ../build/kernel-x86_64-qemu mntPoint/
cd ..
qemu-system-x86_64 -serial stdio -machine accel=tcg -smp "" -drive format=raw,file=/home/zyf/os-workbench/kernel/build/kernel-x86_64-qemu