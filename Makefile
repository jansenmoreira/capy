.PHONY: linux/debug linux/release rapidhash coverage

CC_FLAGS := -Iinclude -std=c99 -Werror -Wall -Wextra -Wconversion -Wpedantic -Wmissing-prototypes -Wmissing-variable-declarations -Wno-missing-field-initializers -Wno-unused-function

LINUX_FLAGS := -DCAPY_LINUX -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L

LINUX_DEBUG_FLAGS := ${CC_FLAGS} ${LINUX_FLAGS} -g -fprofile-arcs -ftest-coverage

LINUX_RELEASE_FLAGS := ${CC_FLAGS} ${LINUX_FLAGS} -DNDEBUG -O3

CC := clang

linux/debug:
	rm -rf build/
	mkdir -p build/
	${CC} ${LINUX_DEBUG_FLAGS} -c src/capy.c -o build/capy.o
	ar rcs build/libcapy.a build/capy.o
	${CC} ${LINUX_DEBUG_FLAGS} -Lbuild tests/test.c -lcapy -o build/tests
	${CC} ${LINUX_DEBUG_FLAGS} -Lbuild examples/echo.c -lcapy -o build/ex_echo

linux/release:
	rm -rf build/release/
	mkdir -p build/release/
	${CC} ${LINUX_RELEASE_FLAGS} -c src/capy.c -o build/release/capy.o
	${CC} ${LINUX_RELEASE_FLAGS} -S src/capy.c -o build/release/capy.s
	ar rcs build/release/libcapy.a build/release/capy.o
	${CC} ${LINUX_RELEASE_FLAGS} -Lbuild/release tests/test.c -lcapy -o build/release/tests
	${CC} ${LINUX_RELEASE_FLAGS} -Lbuild/release examples/echo.c -lcapy -o build/release/ex_echo

rapidhash:
	cd src/ && \
	curl -fsSLO https://github.com/Nicoshev/rapidhash/raw/refs/heads/master/rapidhash.h

coverage:
	gcovr \
		--html-theme green \
		--html-details build/coverage.html \
		--exclude tests/ \
		--exclude examples/ \
		--exclude include/capy/test.h \
		--exclude src/assert.c \
		--gcov-executable "llvm-cov gcov"
	echo file://$$(readlink -f build/coverage.html)
