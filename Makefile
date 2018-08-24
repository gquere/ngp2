CC = clang

# FLAGS ########################################################################
CFLAGS = -I./include/ -Wall -Wextra -Wpedantic -O3 -ggdb3
LDFLAGS = -lpthread -lncurses

# TARGETS ######################################################################
all: build

build: ngp

clean:
	rm -f ngp ngp_perf

install:
	cp ngp /usr/local/bin/ngp

ngp: ./src/*.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

ngp_perf: ./src/*.c
	$(CC) $(CFLAGS) -D_PERFORMANCE_TEST $^ -o $@ $(LDFLAGS)

perf: ngp_perf


# TEST #########################################################################
test: check

check: ngp_perf
	cd test && pwd && for test in ./test_*.sh; do $$test; done
