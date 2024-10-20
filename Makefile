PSYQ_VER := 470
PSYQ_SDK := psyq_sdk/psyq$(PSYQ_VER)/LIB/LIBGPU.LIB

.PHONY: all
all: nugget/psyq/lib/libgpu.a nugget/psyq/include/libgpu.h
clean:
	cd psyq2elf && git clean -xfd
	cd nugget && git clean -xfd

nugget/psyq/lib/libgpu.a: $(PSYQ_SDK) psyq2elf/psyqdump bin/psyq-obj-parser:
	cd psyq2elf && ../bin/psyqlib.sh ../psyq_sdk/psyq$(PSYQ_VER)/LIB ../nugget/psyq/lib
nugget/psyq/include/libgpu.h: $(PSYQ_SDK) psyq2elf/psyqdump
	cd psyq2elf && ./psyqinc.sh ../psyq_sdk/psyq$(PSYQ_VER)/INCLUDE ../nugget/psyq/include
$(PSYQ_SDK):
	cd psyq_sdk && make all
psyq2elf/psyqdump:
	cd psyq2elf && make
bin/psyq-obj-parser:
	