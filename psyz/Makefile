BUILD_DIR := build

build: $(BUILD_DIR)/libpsyz.a

$(BUILD_DIR)/libpsyz.a: CMakeLists.txt
	cmake -GNinja -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

format:
	find src psxgen tests -type f -name '*.c' -exec clang-format -i {} +
