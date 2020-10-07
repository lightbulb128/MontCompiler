g++ montLexer.cpp montParser.cpp montConceiver.cpp montAssembler.cpp montCompiler.cpp -o test
./test testcode.txt >testcode.s
riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O3 testcode.s -o testcode.out
qemu-riscv32 testcode.out
echo $?