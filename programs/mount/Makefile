SHELL=/bin/bash

EXEC_NAME=mount
BUILD_PATH=/tmp/ReOS/programs/$(EXEC_NAME)/debug

.PHONY: default clean

default: $(BUILD_PATH)/$(EXEC_NAME)

include ../../cpp.mk

$(BUILD_PATH)/%.cpp.o: src/%.cpp
	@ mkdir -p $(BUILD_PATH)
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Compile (program) $(FILE_COLOR)$<$(NO_COLOR)"
	@ $(CXX) -c $< -o $@ $(PROGRAM_FLAGS)

$(BUILD_PATH)/$(EXEC_NAME): $(BUILD_PATH)/main.cpp.o
	@ echo -e "$(MODE_COLOR)[debug]$(NO_COLOR) Link (program) $(FILE_COLOR)$<$(NO_COLOR)"
	@ $(CXX) -o $(BUILD_PATH)/$(EXEC_NAME) $(BUILD_PATH)/main.cpp.o $(PROGRAM_LINK_FLAGS)

clean:
	rm -rf $(BUILD_PATH)
