BUILD_DIR := build

build: $(BUILD_DIR)/libpsyz.a

$(BUILD_DIR)/libpsyz.a: CMakeLists.txt
	cmake -GNinja -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
