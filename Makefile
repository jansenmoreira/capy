.PHONY: debug tests-debug

CLANG_FLAGS := -Iinclude -std=c11 -fprofile-arcs -ftest-coverage -Werror -Wall -Wextra -Wconversion -Wpedantic -Wmissing-prototypes -Wmissing-variable-declarations -Wno-missing-field-initializers -Wno-extra-semi
LINUX_FLAGS := -DCAPY_LINUX -D_POSIX_C_SOURCE=200112L

rapidhash:
	cd src/ && \
	curl -fsSLO https://github.com/Nicoshev/rapidhash/raw/refs/heads/master/rapidhash.h

debug:
	rm -rf build/
	mkdir -p build/
	clang -g ${CLANG_FLAGS} ${LINUX_FLAGS} -c src/capy.c -o build/capy.o
	ar rcs build/libcapy.a build/capy.o

tests: debug
	clang -g ${CLANG_FLAGS} -Lbuild tests/test.c -lcapy -o build/tests
	clang -g ${CLANG_FLAGS} ${LINUX_FLAGS} -Lbuild tests/test_api.c -lcapy -o build/test_api

release:
	rm -rf build/release/
	mkdir -p build/release/
	clang -g ${CLANG_FLAGS} ${LINUX_FLAGS} -c src/capy.c -o build/release/capy.o
	ar rcs build/release/libcapy.a build/capy.o
	clang -g ${CLANG_FLAGS} ${LINUX_FLAGS} -O3 -Lbuild/release tests/test_api.c -lcapy -o build/release/test_api


coverage:
	gcovr \
		--html-theme green \
		--html-details build/coverage.html \
		--exclude tests/ \
		--gcov-executable "llvm-cov gcov"
	echo file://$$(readlink -f build/coverage.html)
