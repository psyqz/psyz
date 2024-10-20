TARGETS := $(wildcard */)

build: build_psx

clean: clean_psx

.PHONY: build_psx
build_psx:
	for dir in $(TARGETS); do \
		cd "$$dir" && make && cd ..; \
	done

.PHONY: clean_psx
clean_psx:
	for dir in $(TARGETS); do \
		cd "$$dir" && make clean && cd ..; \
	done

.PHONY: build_pc
build_pc:
	for dir in $(TARGETS); do \
		cd "$$dir" && cmake -GNinja -B build/  && cmake --build build/ && cd ..; \
	done

clean_pc:
	for dir in $(TARGETS); do \
		cd "$$dir" && (rm -rf build/ || true) && cd ..; \
	done
