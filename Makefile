# Convenience Makefile for Linux/macOS and Windows (via existing presets)

.PHONY: all configure build run clean windows linux

all: build

configure:
	cmake -S . -B build

build: configure
	cmake --build build --config Release -- -j

run:
	@if [ -f build/zask ]; then \
		./build/zask; \
	elif [ -f build/Release/zask.exe ]; then \
		build/Release/zask.exe; \
	else \
		echo "No binary found. Build first."; \
	fi

clean:
	cmake -E rm -rf build

# Linux target using system wxWidgets packages
linux:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build --config Release -- -j

# Windows target expects vcpkg + preset
windows:
	cmake --preset windows-vs2022
	cmake --build --preset windows-vs2022-release


