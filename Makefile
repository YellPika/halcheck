.PHONY: all configure test tidy clean

TYPE ?= Release
STD ?= 11
EXT ?= 0

all: build/build.ninja
	@ninja -C build

build/build.ninja: Makefile
	$(MAKE) configure

configure:
	@mkdir -p build
	cmake -B build -GNinja \
		-DCMAKE_BUILD_TYPE=$(TYPE) \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DCMAKE_CXX_FLAGS=$(CXXFLAGS) \
		-DCMAKE_CXX_STANDARD=$(STD) \
		-DCMAKE_CXX_EXTENSIONS=$(EXT) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1

test:
	ctest --test-dir build

tidy: build/build.ninja
	find src -type f | xargs clang-tidy -p build/compile_commands.json

format:
	find include src -type f | xargs clang-format -i

clean:
	@ninja -C build clean
