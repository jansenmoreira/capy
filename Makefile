CC_FLAGS := -Iinclude -std=c11 -Werror -Wall -Wextra -Wconversion -Wpedantic -Wmissing-prototypes -Wmissing-variable-declarations -Wno-missing-field-initializers -Wno-unused-function -Wno-implicit-fallthrough
LINUX_FLAGS := -DCAPY_OS_LINUX -DCAPY_ARCH_AMD64 -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L
LINUX_DEBUG_FLAGS := ${CC_FLAGS} ${LINUX_FLAGS} -g -fprofile-arcs -ftest-coverage
LINUX_RELEASE_FLAGS := ${CC_FLAGS} ${LINUX_FLAGS} -DNDEBUG -O3

CC := gcc

.PHONY: linux/debug
linux/debug:
	rm -rf build/debug/
	mkdir -p build/debug/
	${CC} ${LINUX_DEBUG_FLAGS} -c src/capy.c -o build/debug/capy.o
	ar rcs build/debug/libcapy.a build/debug/capy.o
	${CC} ${LINUX_DEBUG_FLAGS} -Lbuild/debug tests/test.c -lcapy -o build/debug/tests
	${CC} ${LINUX_DEBUG_FLAGS} -Lbuild/debug examples/echo.c -lcapy -o build/debug/ex_echo

.PHONY: linux/debug/ssl
linux/debug/ssl:
	rm -rf build/debug/
	mkdir -p build/debug/
	${CC} ${LINUX_DEBUG_FLAGS} -DCAPY_OPENSSL -c src/capy.c -o build/debug/capy.o
	ar rcs build/debug/libcapy.a build/debug/capy.o
	${CC} ${LINUX_DEBUG_FLAGS} -Lbuild/debug tests/test.c -lssl -lcrypto -lcapy -o build/debug/tests
	${CC} ${LINUX_DEBUG_FLAGS} -Lbuild/debug examples/echo.c -lssl -lcrypto -lcapy -o build/debug/ex_echo

.PHONY: linux/release
linux/release:
	rm -rf build/release/
	mkdir -p build/release/
	${CC} ${LINUX_RELEASE_FLAGS} -c src/capy.c -o build/release/capy.o
	ar rcs build/release/libcapy.a build/release/capy.o
	${CC} ${LINUX_RELEASE_FLAGS} -Lbuild/release tests/test.c -lcrypto -lssl -lcapy -o build/release/tests
	${CC} ${LINUX_RELEASE_FLAGS} -Lbuild/release examples/echo.c -lcrypto -lssl -lcapy -o build/release/ex_echo

.PHONY: linux/musl
linux/musl:
	rm -rf build/musl/
	mkdir -p build/musl/
	musl-gcc -static ${LINUX_RELEASE_FLAGS} -c src/capy.c -o build/musl/capy.o
	ar rcs build/musl/libcapy.a build/musl/capy.o
	musl-gcc -static ${LINUX_RELEASE_FLAGS} -Lbuild/musl tests/test.c -lcapy -o build/musl/tests
	musl-gcc -static ${LINUX_RELEASE_FLAGS} -Lbuild/musl examples/echo.c -lcapy -o build/musl/ex_echo

.PHONY: rapidhash
rapidhash:
	cd src/ && \
	curl -fsSLO https://github.com/Nicoshev/rapidhash/raw/refs/heads/master/rapidhash.h

.PHONY: coverage
coverage:
	gcovr \
		--html-theme green \
		--html-details build/debug/coverage.html \
		--exclude tests/ \
		--exclude examples/ \
		--exclude include/capy/test.h \
		--exclude src/assert.c \
		--gcov-executable "llvm-cov gcov"
	echo file://$$(readlink -f build/debug/coverage.html)

.PHONY: certificates
certificates:
	rm -rf extra/certificates/
	mkdir -p extra/certificates/
	cd extra/certificates/ && \
	openssl genrsa > server_key.pem && \
	openssl req -new -x509 -key server_key.pem > server_chain.pem
