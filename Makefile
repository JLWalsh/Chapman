
quickbuild: build
	cd build && cmake ../

build: 
	mkdir -p build

.PHONY: clean
format:
	find src -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;

