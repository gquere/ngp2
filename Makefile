CC = gcc

# SEARCH ALGORITHM #############################################################
ALGO ?= -D_BMH


# FLAGS ########################################################################
CFLAGS = -I./include/ -Wall -Wextra -Wpedantic -Wno-unused-function -O3
CFLAGS += $(ALGO)
LDFLAGS = -lpthread -lncurses


# TARGETS ######################################################################
all: build

build: ngp

clean:
	rm -f ngp ngp_perf

install: build
	cp ngp /usr/local/bin/ngp

ngp: ./src/*.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

ngp_perf: ./src/*.c
	$(CC) $(CFLAGS) -D_PERFORMANCE_TEST $^ -o $@ $(LDFLAGS)

perf: ngp_perf


# TESTS ########################################################################
test: check

check: ngp_perf
	cd test && pwd && for test in ./test_*.sh; do $$test; done
