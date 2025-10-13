TARGET = test
TYPE = ps-exe
PCSX = $$(cat .emu)

SRCS = $(TARGET).c \
../../nugget/common/crt0/crt0.s \

include ../../psyz/psx.mk

.PHONY: gen
gen: all
	git checkout pcsx.json
	python3 ./psxgen.py $(PCSX) $(TARGET).$(TYPE)
	git checkout pcsx.json
