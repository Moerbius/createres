.PHONY: all clean

CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -pedantic -DHAVE_CONFIG_H

# Detect platform
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM := linux
endif
ifeq ($(OS),Windows_NT)
    PLATFORM := win32
endif
ifndef PLATFORM
    $(warning Platform detection inconclusive, defaulting to linux)
    PLATFORM := linux
endif

SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
TARGET := $(BUILD_DIR)/createres

SOURCES := \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/Resource.cpp \
	$(SRC_DIR)/snappy/snappy.cc \
	$(SRC_DIR)/snappy/snappy-sinksource.cc \
	$(SRC_DIR)/snappy/snappy-stubs-internal.cc

OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter %.cpp,$(SOURCES)))
OBJECTS += $(patsubst $(SRC_DIR)/%.cc,$(OBJ_DIR)/%.o,$(filter %.cc,$(SOURCES)))

CPPFLAGS ?= -I$(SRC_DIR)/snappy

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJECTS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR): | $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: help
help:
	@echo "createres Makefile"
	@echo ""
	@echo "Usage:"
	@echo "  make                Build for current platform (detected: $(PLATFORM))"
	@echo "  make clean          Clean build artifacts"
	@echo "  make linux          Force Linux build"
	@echo "  make win32          Force Windows build"
	@echo ""
	@echo "Variables:"
	@echo "  CXX                 C++ compiler (default: g++)"
	@echo "  CXXFLAGS            Compiler flags"

.PHONY: linux
linux:
	$(MAKE) CPPFLAGS="-I$(SRC_DIR)/snappy" CXXFLAGS="-std=c++11 -Wall -Wextra -pedantic -DHAVE_CONFIG_H"

.PHONY: win32
win32:
	$(MAKE) CPPFLAGS="-I$(SRC_DIR)/snappy -DWIN32_CONFIG" CXXFLAGS="-std=c++11 -Wall -Wextra -pedantic -DHAVE_CONFIG_H"