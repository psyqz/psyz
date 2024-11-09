#/bin/sh
mipsel-linux-gnu-gcc -O0 -g -o psyq$1.elf -T psyq.ld \
    -fno-toplevel-reorder -nostartfiles -nostdlib \
    --whole-file -rdynamic -Wl,--whole-archive \
    -Wl,-Map=psyq$1.map $(find obj/$1/lib* -name '*.o' | sort)
