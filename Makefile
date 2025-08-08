# Convenience Makefile that wraps CMake

BUILD_DIR ?= build/linux
WINDOWS_BUILD_DIR ?= build/windows-x86_64
OS := $(shell uname -s | tr '[:upper:]' '[:lower:]')
ARCH := $(shell uname -m | tr '[:upper:]' '[:lower:]')
BIN := $(BUILD_DIR)/media-converter-$(OS)-$(ARCH)

.PHONY: all configure build run clean windows win-config win-build install

all: build

configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

build: configure
	cmake --build $(BUILD_DIR) --parallel

run: build
	$(BIN)

clean:
	rm -rf build

windows: win-config win-build

win-config:
	cmake -S . -B $(WINDOWS_BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=toolchains/mingw-w64-x86_64.cmake

win-build:
	cmake --build $(WINDOWS_BUILD_DIR) --parallel

install: build
	cmake --install $(BUILD_DIR)


