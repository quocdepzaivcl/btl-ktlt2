CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
SRC_DIR := src
USE_CASES_DIR := $(SRC_DIR)/use_cases
INCLUDE_DIR := include
BIN_DIR := bin
BUILD_DIR := build

SRCS := $(SRC_DIR)/main.cpp \
        $(SRC_DIR)/Application.cpp \
        $(wildcard $(USE_CASES_DIR)/*.cpp)

OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

TARGET := $(BIN_DIR)/main
TEST_TARGET := $(BIN_DIR)/test_runner
UC_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(wildcard $(USE_CASES_DIR)/*.cpp))

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(BUILD_DIR)/test_runner.o $(UC_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/test_runner.o: tests/test_runner.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR) $(BUILD_DIR)/use_cases
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(BUILD_DIR) $(BUILD_DIR)/use_cases:
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)

