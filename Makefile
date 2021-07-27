.PHONY: format setup

quickbuild: build
	cd build && cmake ../

build: 
	mkdir -p build

setup:
	git submodule init
	git submodule update

format:
	find src -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;

