FLAGS_CC := \
	-Iinclude \
	-std=c11 \
	-Werror \
	-Wall \
	-Wextra \
	-Wconversion \
	-Wpedantic \
	-Wmissing-prototypes \
	-Wmissing-variable-declarations \
	-Wno-missing-field-initializers \
	-Wno-unused-function \
	-Wno-implicit-fallthrough

FLAGS_LINUX := \
	-DCAPY_OS_LINUX \
	-DCAPY_ARCH_AMD64 \
	-D_GNU_SOURCE \
	-D_POSIX_C_SOURCE=200809L

LIBS := \
	-lcapy

CC := gcc


.PHONY: linux/build
linux/build:
	rm -rf ${TARGET}
	mkdir -p ${TARGET}
	${CC} ${FLAGS} -c src/capy.c -o ${TARGET}/capy.o
	ar rcs ${TARGET}/libcapy.a ${TARGET}/capy.o
	${CC} ${FLAGS} tests/test.c    -L${TARGET} ${LIBS} -o ${TARGET}/tests
	${CC} ${FLAGS} examples/echo.c -L${TARGET} ${LIBS} -o ${TARGET}/ex_echo


.PHONY: linux/debug
linux/debug: FLAGS  := ${FLAGS_CC} ${FLAGS_LINUX} -g -fprofile-arcs -ftest-coverage
linux/debug: TARGET := build/debug
linux/debug: linux/build


.PHONY: linux/release
linux/release: FLAGS  := ${FLAGS_CC} ${FLAGS_LINUX} -DNDEBUG -O3
linux/release: TARGET := build/release
linux/release: linux/build


.PHONY: linux/debug/ssl
linux/debug/ssl: FLAGS  := ${FLAGS_CC} ${FLAGS_LINUX} -DCAPY_OPENSSL -g -fprofile-arcs -ftest-coverage
linux/debug/ssl: TARGET := build/debug
linux/debug/ssl: LIBS   += -lssl -lcrypto
linux/debug/ssl: linux/build


.PHONY: linux/release/ssl
linux/release/ssl: FLAGS  := ${FLAGS_CC} ${FLAGS_LINUX} -DCAPY_OPENSSL -DNDEBUG -O3
linux/release/ssl: TARGET := build/release
linux/release/ssl: LIBS   += -lssl -lcrypto
linux/release/ssl: linux/build


.PHONY: linux/musl
linux/musl: FLAGS  := ${FLAGS_CC} ${FLAGS_LINUX} -DNDEBUG -O3 -static -Wno-unused-command-line-argument
linux/musl: TARGET := build/musl
linux/musl: CC     := musl-clang
linux/musl: linux/build


compile_flags.txt: FLAGS := ${FLAGS_CC} ${FLAGS_LINUX} -DCAPY_OPENSSL
compile_flags.txt:
	echo "${FLAGS}" | tr ' ' '\n' > compile_flags.txt


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

.PHONY: update/rapidhash
update/rapidhash:
	mkdir -p include/rapidhash
	cd include/rapidhash && \
	curl -fsSLO https://github.com/Nicoshev/rapidhash/raw/refs/heads/master/rapidhash.h && \
	curl -fsSLO https://github.com/Nicoshev/rapidhash/raw/refs/heads/master/LICENSE
