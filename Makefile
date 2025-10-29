FLAGS_CC := \
	-Iinclude \
	-std=c11 \
	-Werror \
	-Wall \
	-Wextra \
	-Wconversion \
	-Wpedantic \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wno-missing-field-initializers \
	-Wno-implicit-fallthrough

FLAGS_LINUX := \
	-D_GNU_SOURCE

LIBS := \
	-lcapy

CC := gcc


all: linux/release


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
linux/debug: LIBS   += -lssl -lcrypto
linux/debug: linux/build


.PHONY: linux/release
linux/release: FLAGS  := ${FLAGS_CC} ${FLAGS_LINUX} -DNDEBUG -O3
linux/release: TARGET := build/release
linux/release: LIBS   += -lssl -lcrypto
linux/release: linux/build


.PHONY:
podman/linux:
	podman image build --tag capy:linux -f contrib/Dockerfile  .
	podman run -it -v ./:/capy capy:linux /bin/bash -c "make $(TARGET) CC=gcc"


.PHONY:
podman/linux/debug: TARGET := linux/debug
podman/linux/debug: podman/linux


.PHONY:
podman/linux/release: TARGET := linux/release
podman/linux/release: podman/linux


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
	rm -rf build/certificates/
	mkdir -p build/certificates/
	cd build/certificates/ && \
	openssl genrsa > server_key.pem && \
	openssl req -new -x509 -key server_key.pem -subj "/O=capy" > server_chain.pem


.PHONY: update/rapidhash
update/rapidhash:
	mkdir -p src/rapidhash
	cd src/rapidhash && \
	curl -fsSLO https://github.com/Nicoshev/rapidhash/raw/refs/heads/master/rapidhash.h && \
	curl -fsSLO https://github.com/Nicoshev/rapidhash/raw/refs/heads/master/LICENSE


vscode:
	mkdir -p .vscode/
	cp contrib/vscode/launch.json .vscode/launch.json
	cp contrib/vscode/tasks.json .vscode/tasks.json
	echo "${FLAGS_CC} ${FLAGS_LINUX} -Wno-unused-includes" | tr ' ' '\n' > compile_flags.txt
