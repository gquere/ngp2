# FLAGS ########################################################################
CFLAGS = -I./include/ -Wall -Wextra -O3
LDFLAGS = -lpthread -lncurses

# TARGETS ######################################################################
all: build

build: ngp

clean:
	rm -f ngp

ngp: ./src/*.c
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)


ngp_perf: ./src/*.c
	gcc $(CFLAGS) -D_PERFORMANCE_TEST $^ -o $@ $(LDFLAGS)
